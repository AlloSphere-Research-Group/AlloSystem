#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
from multiprocessing import Process, Pipe, Array, Queue
#from time import sleep

import sys
import os
import signal

global local_procs
local_procs = []

from appnode import BuildNode, RunNode

        
def run_node(index, stdout, stderr, status, node):
    
    if type(node) == BuildNode:
        node.stdout_cb = lambda text: stdout.send(text)
        node.stderr_cb = lambda text: stderr.send(text)
        node.build()
        if not build_ret == 0:
            status[index] = 1
        else:
            status[index] = 2
        
        return build_ret
    else:
        m = BuildMachine(**node_conf)
        build_ret = m.build(lambda text: stdout.send(text),
                            lambda text: stderr.send(text))
        if not build_ret == 0:
            status[index] = 1
        else:
            status[index] = 2
    #        
    #        print("Output for %s:"%builder["hostname"])
    #        print(m.stdout)
    #        print("Errors for %s:"%builder["hostname"])
    #        print(m.stderr)
    #    else:
    #        print('OK: ' + str(index) +  ' ' + str(build_ret))
        return build_ret

# Curses GUI
import curses

class CursesApp():
    def __init__(self, nodes, source):
        self.current_index = 0
        self.buffer_len = 128
        self.chase = True
        if nodes and len(nodes) > 0:
            self.nodes = nodes

        self.item_status = Array('i', [4 for i in self.nodes])
        #self.item_color = { "none": 7, "error" : 1, "done" : 2, "working" : 4}

        self.displays = ["STDOUT", "STDERR"]
        self.current_display = 0
        self.log_text = ''
        self.source = source
        pass
    
    def log(self, text):
        self.log_text += text + '\n'
    
    def draw_menu(self, stdscr, linenum):
        pos = 1
        for node in self.nodes:
            color = self.item_status[self.nodes.index(node)]
            style = curses.A_REVERSE if self.nodes.index(node) == self.current_index else curses.A_NORMAL
            stdscr.addstr(0,pos, node.name, curses.color_pair(color) | style)
            pos += len(node.name) + 1
        y, x = stdscr.getmaxyx()
        stdscr.addstr(y - 1, 1, "q:Quit  space:Switch  c:Chase Output", curses.A_NORMAL)
        
        stdscr.addstr(y - 1, x-18, "L%03i"%linenum,  curses.A_REVERSE)
        stdscr.addstr(y - 1, x-8, self.displays[self.current_display], curses.A_REVERSE)
        stdscr.refresh()
        

    def draw_run_menu(self, stdscr, linenum):
        pos = 1
        for runner in self.runners:
            color = 1
            style = curses.A_REVERSE if self.runners.index(runner) == self.current_index else curses.A_NORMAL
            stdscr.addstr(0,pos, runner.name, curses.color_pair(color) | style)
            pos += len(runner.name) + 1
        y, x = stdscr.getmaxyx()
        stdscr.addstr(y - 1, 1, "q:Quit  space:Switch  c:Chase Output", curses.A_NORMAL)
        
        stdscr.addstr(y - 1, x-18, "L%03i"%linenum,  curses.A_REVERSE)
        stdscr.addstr(y - 1, x-8, self.displays[self.current_display], curses.A_REVERSE)
        stdscr.refresh()        
        
    def wrap_lines(self, text, stdscr):
        y, x = stdscr.getmaxyx()
        wrapped = []
        for line in text.splitlines():
            while len(line) >= x - 2:
                wrapped.append(line[:x - 2])
                line = line[x - 2:]
            wrapped.append(line)
        return '\n'.join(wrapped)
    
    def app_loop(self, stdscr):
        global local_procs
        curses.halfdelay(1)
        curses.start_color()
        curses.init_pair(4, curses.COLOR_BLUE, curses.COLOR_BLACK)
        curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)
        curses.init_pair(7, curses.COLOR_WHITE, curses.COLOR_BLACK)
        curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)

        try:
            curses.curs_set(0)
        except curses.error:
            pass
        
        self.pad = curses.newpad(self.buffer_len, 512)
        self._run_builders(stdscr)

        self._start_runners(stdscr)
        
    def _run_builders(self, stdscr):
        #processes = []
        self.stdout  = [Pipe() for i in self.nodes]
        self.stderr = [Pipe() for i in self.nodes]
        self.stdoutbuf  = ['' for i in self.nodes]
        self.stderrbuf = ['' for i in self.nodes]
        for i, node in enumerate(self.nodes):
            #if node["build"]:
            q = Queue() 
            node.pid_queue = q
            
            builder = node
            # These tests assume you are running from an AlloProject directory
            # that contains an AlloSystem repo
            configuration = {"project_dir" : '',
                            "project_src": self.source,
                            "prebuild_commands" : '',
                            "build_command" : './run.sh '
                            }
            builder.configure(**configuration)
            builder.stdout_cb = lambda text: self.stdout[i][1].send(text)
            builder.stderr_cb = lambda text: self.stderr[i][1].send(text)
            builder.build()
            
        mypad_pos = 0
        num_lines = 0
        while not builder.is_done():
            y, x = stdscr.getmaxyx()
            if mypad_pos < 0:
                mypad_pos = 0
            if mypad_pos > self.buffer_len - y:
                mypad_pos = self.buffer_len - y
            self.draw_menu(stdscr, mypad_pos + 1)
            self.pad.erase()
            
            builder.flush_messages()
            while self.stdout[self.current_index][0].poll():
                self.stdoutbuf[self.current_index] += self.stdout[self.current_index][0].recv()
                
            while self.stderr[self.current_index][0].poll():
                self.stderrbuf[self.current_index] += self.stderr[self.current_index][0].recv()
               
            if self.current_display == 0:
                console_output = self.stdoutbuf[self.current_index]
                color = curses.color_pair(2)
            elif self.current_display == 1:
                console_output = self.stderrbuf[self.current_index]
                color = curses.color_pair(7)
            
            if not '\n' in console_output:
                console_output += '\n' # Needed otherwise curses crashes!
             
            console_output = self.wrap_lines(console_output, stdscr)
            num_lines = console_output.count('\n') + 1
            if num_lines >= self.buffer_len:
                lines = console_output.split('\n')
                console_output = '\n'.join(lines[len(lines) - self.buffer_len:])
                
                num_lines = console_output.count('\n')
                
            self.pad.addstr(0,0, console_output, color)
            
            self.pad.refresh(mypad_pos, 0, 2, 0, y - 4, x - 3)
            
            try:
                event = stdscr.getch()
                if event == ord('q'): break
                elif event == ord('c'): self.chase = not self.chase
                elif event == curses.KEY_LEFT:
                    self.current_index -= 1
                    if self.current_index < 0:
                        self.current_index = len(self.nodes) - 1
                elif event == curses.KEY_RIGHT:
                    self.current_index += 1
                    if self.current_index >= len(self.nodes) :
                        self.current_index = 0
                elif event == curses.KEY_DOWN:
                    mypad_pos += 1
                elif event == curses.KEY_UP:
                    mypad_pos -= 1 
                elif event == curses.KEY_PPAGE:
                    mypad_pos -= y
                elif event == curses.KEY_NPAGE:
                    mypad_pos += y
                elif event == ord(' '):
                    self.current_display += 1
                    if self.current_display >= len(self.displays):
                        self.current_display = 0

            except:
                pass


        builder.flush_messages()
        for pid in local_procs:
            try:
               os.kill(pid, signal.SIGTERM)
            except:
               self.log("Could not kill pid: %i"%pid)

        if self.chase:
            mypad_pos = num_lines - y
#        for p in processes:
#            self.item_status
#            p.terminate()
            
    def _start_runners(self, stdscr):
                #processes = []
        self.runstdout  = []
        self.runstderr = []
        self.runstdoutbuf  = []
        self.runstderrbuf = []
        self.runners = []
        for i, node in enumerate(self.nodes):
            #if node["build"]:
            q = Queue() 
            node.pid_queue = q
            build_report = 'build/' + self.source[:-4].replace('/', '_') + '.json'
            import json
            with open(build_report) as fp:
                conf = json.load(fp)    
                for app in conf['apps']:
                    self.runstdout.append(Pipe())
                    self.runstderr.append(Pipe())
                    self.runstdoutbuf.append('')
                    self.runstderrbuf.append('')
                    runner = RunNode(app['type'] + '(local)',
                                     lambda text: self.runstdout[-1][1].send(text),
                                     lambda text: self.runstderr[-1][1].send(text))
                    bin_path = conf['bin_dir'] + app['path']
                    runner.configure(conf['root_dir'], bin_path)
                    runner.run()
                    self.runners.append(runner)

        mypad_pos = 0
        num_lines = 0
        while True:
            y, x = stdscr.getmaxyx()
            if mypad_pos < 0:
                mypad_pos = 0
            if mypad_pos > self.buffer_len - y:
                mypad_pos = self.buffer_len - y
            self.draw_run_menu(stdscr, mypad_pos + 1)
            self.pad.erase()
            
            #runner.flush_messages()
            while self.runstdout[self.current_index][0].poll():
                self.runstdoutbuf[self.current_index] += self.runstdout[self.current_index][0].recv()
                
            while self.runstderr[self.current_index][0].poll():
                self.runstderrbuf[self.current_index] += self.runstderr[self.current_index][0].recv()
               
            if self.current_display == 0:
                console_output = self.runstdoutbuf[self.current_index]
                color = curses.color_pair(2)
            elif self.current_display == 1:
                console_output = self.runstderrbuf[self.current_index]
                color = curses.color_pair(7)
            
            if not '\n' in console_output:
                console_output += '\n' # Needed otherwise curses crashes!
             
            console_output = self.wrap_lines(console_output, stdscr)
            num_lines = console_output.count('\n') + 1
            if num_lines >= self.buffer_len:
                lines = console_output.split('\n')
                console_output = '\n'.join(lines[len(lines) - self.buffer_len:])
                
                num_lines = console_output.count('\n')
                
            self.pad.addstr(0,0, console_output, color)
            
            self.pad.refresh(mypad_pos, 0, 2, 0, y - 4, x - 3)
            
            try:
                event = stdscr.getch()
                if event == ord('q'): break
                elif event == ord('c'): self.chase = not self.chase
                elif event == curses.KEY_LEFT:
                    self.current_index -= 1
                    if self.current_index < 0:
                        self.current_index = len(self.runners) - 1
                elif event == curses.KEY_RIGHT:
                    self.current_index += 1
                    if self.current_index >= len(self.runners) :
                        self.current_index = 0
                elif event == curses.KEY_DOWN:
                    mypad_pos += 1
                elif event == curses.KEY_UP:
                    mypad_pos -= 1 
                elif event == curses.KEY_PPAGE:
                    mypad_pos -= y
                elif event == curses.KEY_NPAGE:
                    mypad_pos += y
                elif event == ord(' '):
                    self.current_display += 1
                    if self.current_display >= len(self.displays):
                        self.current_display = 0

            except:
                pass

        for pid in local_procs:
            try:
               os.kill(pid, signal.SIGTERM)
            except:
               self.log("Could not kill pid: %i"%pid)

        if self.chase:
            mypad_pos = num_lines - y
        
    def start(self):
        curses.wrapper(self.app_loop)


if __name__ == "__main__":
    import socket
#    import json
    hostname = socket.gethostname()
    if len(sys.argv) > 1:
        project_src = sys.argv[1]
    else:
        project_src = 'AlloSystem/alloutil/examples/allosphereApp.cpp'
    
    if hostname == 'audio.10g':
        from allosphere import nodes
    else:
        from local import nodes
    
    if len(nodes) > 0:
        app = CursesApp(nodes, project_src)
        app.start()
        print('Output log: -------------\n' + app.log_text)
    else:
        print("No nodes. Aborting.")
    

