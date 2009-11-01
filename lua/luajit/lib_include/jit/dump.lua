----------------------------------------------------------------------------
-- LuaJIT compiler dump module.
--
-- Copyright (C) 2005-2009 Mike Pall. All rights reserved.
-- Released under the MIT/X license. See Copyright Notice in luajit.h
----------------------------------------------------------------------------
--
-- This module can be used to debug the JIT compiler itself. It dumps the
-- code representations and structures used in various compiler stages.
--
-- Example usage:
--
--   luajit -jdump -e "local x=0; for i=1,1e6 do x=x+i end; print(x)"
--   luajit -jdump=im -e "for i=1,1000 do for j=1,1000 do end end" | less -R
--   luajit -jdump=is myapp.lua | less -R
--   luajit -jdump=-b myapp.lua
--   luajit -jdump=+aH,myapp.html myapp.lua
--   luajit -jdump=ixT,myapp.dump myapp.lua
--
-- The first argument specifies the dump mode. The second argument gives
-- the output file name. Default output is to stdout, unless the environment
-- variable LUAJIT_DUMPFILE is set. The file is overwritten every time the
-- module is started.
--
-- Different features can be turned on or off with the dump mode. If the
-- mode starts with a '+', the following features are added to the default
-- set of features; a '-' removes them. Otherwise the features are replaced.
--
-- The following dump features are available (* marks the default):
--
--  * t  Print a line for each started, ended or aborted trace (see also -jv).
--  * b  Dump the traced bytecode.
--  * i  Dump the IR (intermediate representation).
--    r  Augment the IR with register/stack slots.
--    s  Dump the snapshot map.
--  * m  Dump the generated machine code.
--    x  Print each taken trace exit.
--    X  Print each taken trace exit and the contents of all registers.
--
-- The output format can be set with the following characters:
--
--    T  Plain text output.
--    A  ANSI-colored text output
--    H  Colorized HTML + CSS output.
--
-- The default output format is plain text. It's set to ANSI-colored text
-- if the COLORTERM variable is set. Note: this is independent of any output
-- redirection, which is actually considered a feature.
--
-- You probably want to use less -R to enjoy viewing ANSI-colored text from
-- a pipe or a file. Add this to your ~/.bashrc: export LESS="-R"
--
------------------------------------------------------------------------------

-- Cache some library functions and objects.
local jit = require("jit")
assert(jit.version_num == 20000, "LuaJIT core/library version mismatch")
local jutil = require("jit.util")
local vmdef = require("jit.vmdef")
local funcinfo, funcbc = jutil.funcinfo, jutil.funcbc
local traceinfo, traceir, tracek = jutil.traceinfo, jutil.traceir, jutil.tracek
local tracemc, traceexitstub = jutil.tracemc, jutil.traceexitstub
local tracesnap = jutil.tracesnap
local bit = require("bit")
local band, shl, shr = bit.band, bit.lshift, bit.rshift
local sub, gsub, format = string.sub, string.gsub, string.format
local byte, char, rep = string.byte, string.char, string.rep
local type, tostring = type, tostring
local stdout, stderr = io.stdout, io.stderr

-- Load other modules on-demand.
local bcline, discreate

-- Active flag, output file handle and dump mode.
local active, out, dumpmode

------------------------------------------------------------------------------

local symtab = {}
local nexitsym = 0

-- Fill symbol table with trace exit addresses.
local function fillsymtab(nexit)
  local t = symtab
  if nexit > nexitsym then
    for i=nexitsym,nexit-1 do t[traceexitstub(i)] = tostring(i) end
    nexitsym = nexit
  end
  return t
end

local function dumpwrite(s)
  out:write(s)
end

-- Disassemble machine code.
local function dump_mcode(tr)
  local info = traceinfo(tr)
  if not info then return end
  local mcode, addr, loop = tracemc(tr)
  if not mcode then return end
  if not discreate then
    discreate = require("jit.dis_"..jit.arch).create
  end
  out:write("---- TRACE ", tr, " mcode ", #mcode, "\n")
  local ctx = discreate(mcode, addr, dumpwrite)
  ctx.hexdump = 0
  ctx.symtab = fillsymtab(info.nexit)
  if loop ~= 0 then
    symtab[addr+loop] = "LOOP"
    ctx:disass(0, loop)
    out:write("->LOOP:\n")
    ctx:disass(loop, #mcode-loop)
    symtab[addr+loop] = nil
  else
    ctx:disass(0, #mcode)
  end
end

------------------------------------------------------------------------------

local irtype_text = {
  [0] = "nil",
  "fal",
  "tru",
  "lud",
  "str",
  "ptr",
  "thr",
  "pro",
  "fun",
  "t09",
  "tab",
  "udt",
  "num",
  "int",
  "i8 ",
  "u8 ",
  "i16",
  "u16",
}

local colortype_ansi = {
  [0] = "%s",
  "%s",
  "%s",
  "%s",
  "\027[32m%s\027[m",
  "%s",
  "\027[1m%s\027[m",
  "%s",
  "\027[1m%s\027[m",
  "%s",
  "\027[31m%s\027[m",
  "\027[36m%s\027[m",
  "\027[34m%s\027[m",
  "\027[35m%s\027[m",
  "\027[35m%s\027[m",
  "\027[35m%s\027[m",
  "\027[35m%s\027[m",
  "\027[35m%s\027[m",
}

local function colorize_text(s, t)
  return s
end

local function colorize_ansi(s, t)
  return format(colortype_ansi[t], s)
end

local irtype_ansi = setmetatable({},
  { __index = function(tab, t)
      local s = colorize_ansi(irtype_text[t], t); tab[t] = s; return s; end })

local html_escape = { ["<"] = "&lt;", [">"] = "&gt;", ["&"] = "&amp;", }

local function colorize_html(s, t)
  s = gsub(s, "[<>&]", html_escape)
  return format('<span class="irt_%s">%s</span>', irtype_text[t], s)
end

local irtype_html = setmetatable({},
  { __index = function(tab, t)
      local s = colorize_html(irtype_text[t], t); tab[t] = s; return s; end })

local header_html = [[
<style type="text/css">
background { background: #ffffff; color: #000000; }
pre.ljdump {
font-size: 10pt;
background: #f0f4ff;
color: #000000;
border: 1px solid #bfcfff;
padding: 0.5em;
margin-left: 2em;
margin-right: 2em;
}
span.irt_str { color: #00a000; }
span.irt_thr, span.irt_fun { color: #404040; font-weight: bold; }
span.irt_tab { color: #c00000; }
span.irt_udt { color: #00c0c0; }
span.irt_num { color: #0000c0; }
span.irt_int { color: #c000c0; }
</style>
]]

local colorize, irtype

-- Lookup table to convert some literals into names.
local litname = {
  ["SLOAD "] = { [0] = "", "I", "R", "RI", "P", "PI", "PR", "PRI", },
  ["XLOAD "] = { [0] = "", "unaligned", },
  ["TOINT "] = { [0] = "check", "index", "", },
  ["FLOAD "] = vmdef.irfield,
  ["FREF  "] = vmdef.irfield,
  ["FPMATH"] = vmdef.irfpm,
}

local function ctlsub(c)
  if c == "\n" then return "\\n"
  elseif c == "\r" then return "\\r"
  elseif c == "\t" then return "\\t"
  elseif c == "\r" then return "\\r"
  else return format("\\%03d", byte(c))
  end
end

local function formatk(tr, idx)
  local k, t, slot = tracek(tr, idx)
  local tn = type(k)
  local s
  if tn == "number" then
    if k == 2^52+2^51 then
      s = "bias"
    else
      s = format("%+.14g", k)
    end
  elseif tn == "string" then
    s = format(#k > 20 and '"%.20s"~' or '"%s"', gsub(k, "%c", ctlsub))
  elseif tn == "function" then
    local fi = funcinfo(k)
    if fi.ffid then
      s = vmdef.ffnames[fi.ffid]
    else
      s = fi.loc
    end
  elseif tn == "table" then
    s = format("{%p}", k)
  elseif tn == "userdata" then
    if t == 11 then
      s = format("userdata:%p", k)
    else
      s = format("[%p]", k)
      if s == "[0x00000000]" then s = "NULL" end
    end
  else
    s = tostring(k) -- For primitives.
  end
  s = colorize(format("%-4s", s), t)
  if slot then
    s = format("%s @%d", s, slot)
  end
  return s
end

local function printsnap(tr, snap)
  for i=1,#snap do
    local ref = snap[i]
    if not ref then
      out:write("---- ")
    elseif ref < 0 then
      out:write(formatk(tr, ref), " ")
    else
      local m, ot, op1, op2 = traceir(tr, ref)
      local t = band(ot, 15)
      local sep = " "
      if t == 8 then
	local oidx = 6*shr(ot, 8)
	local op = sub(vmdef.irnames, oidx+1, oidx+6)
	if op == "FRAME " then
	  sep = "|"
	end
      end
      out:write(colorize(format("%04d", ref), t), sep)
    end
  end
  out:write("]\n")
end

-- Dump snapshots (not interleaved with IR).
local function dump_snap(tr)
  out:write("---- TRACE ", tr, " snapshots\n")
  for i=0,1000000000 do
    local snap = tracesnap(tr, i)
    if not snap then break end
    out:write(format("#%-3d %04d [ ", i, snap[0]))
    printsnap(tr, snap)
  end
end

-- NYI: should really get the register map from the disassembler.
local reg_map = {
  [0] = "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
}

-- Return a register name or stack slot for a rid/sp location.
local function ridsp_name(ridsp)
  local rid = band(ridsp, 0xff)
  if ridsp > 255 then return format("[%x]", shr(ridsp, 8)*4) end
  if rid < 128 then return reg_map[rid] end
  return ""
end

-- Dump IR and interleaved snapshots.
local function dump_ir(tr, dumpsnap, dumpreg)
  local info = traceinfo(tr)
  if not info then return end
  local nins = info.nins
  out:write("---- TRACE ", tr, " IR\n")
  local irnames = vmdef.irnames
  local snapref = 65536
  local snap, snapno
  if dumpsnap then
    snap = tracesnap(tr, 0)
    snapref = snap[0]
    snapno = 0
  end
  for ins=1,nins do
    if ins >= snapref then
      if dumpreg then
	out:write(format("....              SNAP   #%-3d [ ", snapno))
      else
	out:write(format("....        SNAP   #%-3d [ ", snapno))
      end
      printsnap(tr, snap)
      snapno = snapno + 1
      snap = tracesnap(tr, snapno)
      snapref = snap and snap[0] or 65536
    end
    local m, ot, op1, op2, ridsp = traceir(tr, ins)
    local oidx, t = 6*shr(ot, 8), band(ot, 31)
    local op = sub(irnames, oidx+1, oidx+6)
    if op == "LOOP  " then
      if dumpreg then
	out:write(format("%04d ------------ LOOP ------------\n", ins))
      else
	out:write(format("%04d ------ LOOP ------------\n", ins))
      end
    elseif op ~= "NOP   " and (dumpreg or op ~= "RENAME") then
      if dumpreg then
	out:write(format("%04d %-5s ", ins, ridsp_name(ridsp)))
      else
	out:write(format("%04d ", ins))
      end
      out:write(format("%s%s %s %s ",
		       band(ot, 64) == 0 and " " or ">",
		       band(ot, 128) == 0 and " " or "+",
		       irtype[t], op))
      local m1 = band(m, 3)
      if m1 ~= 3 then -- op1 != IRMnone
	if op1 < 0 then
	  out:write(formatk(tr, op1))
	else
	  out:write(format(m1 == 0 and "%04d" or "#%-3d", op1))
	end
	local m2 = band(m, 3*4)
	if m2 ~= 3*4 then -- op2 != IRMnone
	  if m2 == 1*4 then -- op2 == IRMlit
	    local litn = litname[op]
	    if litn and litn[op2] then
	      out:write("  ", litn[op2])
	    else
	      out:write(format("  #%-3d", op2))
	    end
	  elseif op2 < 0 then
	    out:write("  ", formatk(tr, op2))
	  else
	    out:write(format("  %04d", op2))
	  end
	end
      end
      out:write("\n")
    end
  end
  if snap then
    if dumpreg then
      out:write(format("....              SNAP   #%-3d [ ", snapno))
    else
      out:write(format("....        SNAP   #%-3d [ ", snapno))
    end
    printsnap(tr, snap)
  end
end

------------------------------------------------------------------------------

local recprefix = ""
local recdepth = 0

-- Format trace error message.
local function fmterr(err, info)
  if type(err) == "number" then
    if type(info) == "function" then
      local fi = funcinfo(info)
      if fi.ffid then
	info = vmdef.ffnames[fi.ffid]
      else
	info = fi.loc
      end
    end
    err = format(vmdef.traceerr[err], info)
  end
  return err
end

-- Dump trace states.
local function dump_trace(what, tr, func, pc, otr, oex)
  if what == "stop" or (what == "abort" and dumpmode.a) then
    if dumpmode.i then dump_ir(tr, dumpmode.s, dumpmode.r and what == "stop")
    elseif dumpmode.s then dump_snap(tr) end
    if dumpmode.m then dump_mcode(tr) end
  end
  if what == "start" then
    if dumpmode.H then out:write('<pre class="ljdump">\n') end
    out:write("---- TRACE ", tr, " ", what)
    if otr then out:write(" ", otr, "/", oex) end
    local fi = funcinfo(func, pc)
    out:write(" ", fi.loc, "\n")
    recprefix = ""
    reclevel = 0
  elseif what == "stop" or what == "abort" then
    out:write("---- TRACE ", tr, " ", what)
    recprefix = nil
    if what == "abort" then
      local fi = funcinfo(func, pc)
      out:write(" ", fi.loc, " -- ", fmterr(otr, oex), "\n")
    else
      local link = traceinfo(tr).link
      if link == tr then
	link = "loop"
      elseif link == 0 then
	link = "interpreter"
      end
      out:write(" -> ", link, "\n")
    end
    if dumpmode.H then out:write("</pre>\n\n") else out:write("\n") end
  else
    out:write("---- TRACE ", what, "\n\n")
  end
  out:flush()
end

-- Dump recorded bytecode.
local function dump_record(tr, func, pc, depth, callee)
  if depth ~= recdepth then
    recdepth = depth
    recprefix = rep(" .", depth)
  end
  local line = bcline(func, pc, recprefix)
  if dumpmode.H then line = gsub(line, "[<>&]", html_escape) end
  if type(callee) == "function" then
    local fi = funcinfo(callee)
    if fi.ffid then
      out:write(sub(line, 1, -2), "  ; ", vmdef.ffnames[fi.ffid], "\n")
    else
      out:write(sub(line, 1, -2), "  ; ", fi.loc, "\n")
    end
  else
    out:write(line)
  end
  if band(funcbc(func, pc), 0xff) < 16 then -- Write JMP for cond. ORDER BC
    out:write(bcline(func, pc+1, recprefix))
  end
end

------------------------------------------------------------------------------

-- Dump taken trace exits.
local function dump_texit(tr, ex, ngpr, nfpr, ...)
  out:write("---- TRACE ", tr, " exit ", ex, "\n")
  if dumpmode.X then
    local regs = {...}
    for i=1,ngpr do
      out:write(format(" %08x", regs[i]))
      if i % 8 == 0 then out:write("\n") end
    end
    for i=1,nfpr do
      out:write(format(" %+17.14g", regs[ngpr+i]))
      if i % 4 == 0 then out:write("\n") end
    end
  end
end

------------------------------------------------------------------------------

-- Detach dump handlers.
local function dumpoff()
  if active then
    active = false
    jit.attach(dump_texit)
    jit.attach(dump_record)
    jit.attach(dump_trace)
    if out and out ~= stdout and out ~= stderr then out:close() end
    out = nil
  end
end

-- Open the output file and attach dump handlers.
local function dumpon(opt, outfile)
  if active then dumpoff() end

  local colormode = os.getenv("COLORTERM") and "A" or "T"
  if opt then
    opt = gsub(opt, "[TAH]", function(mode) colormode = mode; return ""; end)
  end

  local m = { t=true, b=true, i=true, m=true, }
  if opt and opt ~= "" then
    local o = sub(opt, 1, 1)
    if o ~= "+" and o ~= "-" then m = {} end
    for i=1,#opt do m[sub(opt, i, i)] = (o ~= "-") end
  end
  dumpmode = m

  if m.t or m.b or m.i or m.s or m.m then
    jit.attach(dump_trace, "trace")
  end
  if m.b then
    jit.attach(dump_record, "record")
    if not bcline then bcline = require("jit.bc").line end
  end
  if m.x or m.X then
    jit.attach(dump_texit, "texit")
  end

  if not outfile then outfile = os.getenv("LUAJIT_DUMPFILE") end
  if outfile then
    out = outfile == "-" and stdout or assert(io.open(outfile, "w"))
  else
    out = stdout
  end

  m[colormode] = true
  if colormode == "A" then
    colorize = colorize_ansi
    irtype = irtype_ansi
  elseif colormode == "H" then
    colorize = colorize_html
    irtype = irtype_html
    out:write(header_html)
  else
    colorize = colorize_text
    irtype = irtype_text
  end

  active = true
end

-- Public module functions.
module(...)

on = dumpon
off = dumpoff
start = dumpon -- For -j command line option.

