#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
from multiprocessing import Process, Pipe, Array
from time import sleep

import sys
import subprocess

class BuildMachine:
    def __init__(self, **kwargs):
        self.stdout = ''
        self.stderr = ''
        if kwargs["remote"]:
            self.remote = kwargs["remote"]
            self.hostname = kwargs["hostname"]
            self.gateway = kwargs["gateway"]
            self.login = kwargs["login"]
            self.root_project_dir = kwargs["root_project_dir"]
            self.project_dir = kwargs["project_dir"]
            self.project_file = kwargs["project_file"]
            self.prebuild_commands = kwargs["prebuild_commands"]
            self.build_command = kwargs["build_command"]
        else:
            raise ValueError("Expected 'remote' key to be true.")

    def get_command(self):
        command = "cd %s;"%self.project_dir
        command += "%s %s"%(self.build_command, self.project_file)
        return command
        
    def build(self, stdout_cb = None, stderr_cb = None):
        command = self.get_command()
        if command:
            host = self.gateway
            command = 'ssh %s "%s"'%(self.hostname, command)
        else: 
            host = self.hostname
            
        for pb_command in self.prebuild_commands:
            self.debug_print("Running:" + pb_command)
            ssh = subprocess.Popen(pb_command.split(),
                       shell=False,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
            self.debug_print(ssh.stdout.readlines() )          
            self.debug_print("Returned: " + str(ssh.returncode))  
            ssh.terminate()
        
        
        self.debug_print("Running on " + "%s@%s" % (self.login,host) + ":" + command)
        ssh = subprocess.Popen(["ssh", "%s@%s" % (self.login,host), command],
                       shell=False,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
            
        for line in iter(ssh.stdout.readline, ''):
            self.stdout += line
            if stdout_cb:
                stdout_cb(line)
            
        for line in iter(ssh.stderr.readline, ''):
            self.stderr += line
            if stderr_cb:
                stderr_cb(line)
        
        ssh.wait()
        #print("Done %s"%self.hostname)
        return ssh.returncode
            
    def debug_print(self, text):
        return
        print(text)
   
class LocalRun:
    def __init__(self, **kwargs):
        if not kwargs["remote"]:
            self.remote = kwargs["remote"]
            self.run_dir = kwargs["run_dir"]
            self.path = kwargs["path"]
        else:
            raise ValueError("Expected 'remote' key to be false.")
            
    def run(self, stdout_cb = None, stderr_cb = None):
        command = self.path
        
        stdout_cb("Running " + command + "\n")
        p = subprocess.Popen([command],
                       shell=False,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE,
                       cwd=self.run_dir)
        self.process = p
            
        if p.returncode:
            return p.returncode
        
        while p.poll() is None:
            for line in iter(p.stdout.readline, ''):
                if stdout_cb:
                    stdout_cb(line)
                
            for line in iter(p.stderr.readline, ''):
                if stderr_cb:
                    stderr_cb(line)

        #p.join()

        return p.returncode        


        
def run_node(index, stdout, stderr, status, node_conf):
    if not node_conf['remote']:
        m = LocalRun(**node_conf)
        build_ret = m.run(lambda text: stdout.send(text),
                          lambda text: stderr.send(text))
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
    def __init__(self, nodes):
        self.current_index = 0
        self.buffer_len = 128
        self.chase = True
        if nodes and len(nodes) > 0:
            self.nodes = nodes

        self.item_status = Array('i', [4 for i in self.nodes])
        #self.item_color = { "none": 7, "error" : 1, "done" : 2, "working" : 4}
        self.stdout  = [Pipe() for i in self.nodes]
        self.stderr = [Pipe() for i in self.nodes]
        self.stdoutbuf  = ['' for i in self.nodes]
        self.stderrbuf = ['' for i in self.nodes]
        self.displays = ["STDOUT", "STDERR"]
        self.current_display = 0
        pass
    
    def draw_menu(self, stdscr, linenum):
        pos = 1
        for node in self.nodes:
            color = self.item_status[self.nodes.index(node)]
            style = curses.A_REVERSE if self.nodes.index(node) == self.current_index else curses.A_NORMAL
            stdscr.addstr(0,pos, node['name'], curses.color_pair(color) | style)
            pos += len(node['name']) + 1
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
          
        processes = []
        for i, node in enumerate(self.nodes):
            p = Process(target=run_node, args=(i, self.stdout[i][1], self.stderr[i][1], self.item_status, node))
            processes.append(p)
            p.start()
            sleep(0.1)

        
        mypad = curses.newpad(self.buffer_len, 512)
        mypad_pos = 0

        while True:
            y, x = stdscr.getmaxyx()
            if mypad_pos < 0:
                mypad_pos = 0
            if mypad_pos > self.buffer_len - y:
                mypad_pos = self.buffer_len - y
            self.draw_menu(stdscr, mypad_pos + 1)
            mypad.erase()
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
                
            mypad.addstr(0,0, console_output, color)
            
            mypad.refresh(mypad_pos, 0, 2, 0, y - 4, x - 3)
            
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
            
        if self.chase:
            mypad_pos = num_lines - y
#        for p in processes:
#            self.item_status
#            p.terminate()
            
    def start(self):
        curses.wrapper(self.app_loop)
            


if __name__ == "__main__":
    
    nodes = []
    if len(sys.argv) > 1:
        import json
        with open(sys.argv[1]) as fp:
            conf = json.load(fp)    
            #print(conf)
            for app in conf['apps']:
                node = {'name': app['type'] + '(local)', 'remote': False , 'path' : app['path'], 'run_dir' : conf['run_dir']}
                nodes.append(node)
    else:
        gr_builder = {
            "remote" : True,
            "hostname" : "gr01",
            "gateway" : "wally.mat.ucsb.edu",
            "login" : "sphere",
            "root_project_dir" : "",
            "project_dir" : "andres/AlloSystem",
            "project_file" : "alloaudio-examples/ambiMetering_simulator/",
            "prebuild_commands" : [ "git pull"],
            "build_command" : "./run.sh -n"
        }  
        num_gr_nodes = 12
        for i in range(num_gr_nodes):
            node = gr_builder.copy()
            node['hostname'] = 'gr%02i'%(i+1)
            nodes.append(node)
        
        node = gr_builder.copy()
        node['hostname'] = 'audio'
        nodes.append(node)
        node = gr_builder.copy()
        node['hostname'] = 'z01'
        nodes.append({'name': 'z01', 'remote': True})
        node = gr_builder.copy()
        node['hostname'] = 'z02'
        nodes.append({'name': 'z02', 'remote': True})
     
        
    app = CursesApp(nodes)
    app.start()
    
    