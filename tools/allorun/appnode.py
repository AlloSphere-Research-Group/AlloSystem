# -*- coding: utf-8 -*-
"""
Created on Wed Mar 30 21:28:32 2016

@author: andres
"""


from __future__ import print_function

import exceptions
import subprocess
import os
import sys

class Node:
    def __init__(self,
                 hostname = 'localhost',
                 gateway = "",
                 login = '',
                 ):
        self.remote = False
        self.hostname = hostname
        self.gateway = gateway
        self.login = login
        self.pid_queue = None
        
        self.stdout = ''
        self.stderr = ''
    
class BuildNode(Node):
    def __init__(self, name = 'nodeb', distributed = True,
                    stdout_cb = lambda t: print(t, end=''), stderr_cb = lambda t: print(t, end='')):
        Node.__init__(self)
        
        self.stdout_cb = stdout_cb
        self.stderr_cb = stderr_cb
        self.name = name
        self.build_distributed_app = distributed
        
    def configure(self, **kwargs):
        self.project_dir = kwargs["project_dir"]
        self.project_src = kwargs["project_src"]
        self.prebuild_commands = kwargs["prebuild_commands"]
        self.build_command = kwargs["build_command"]
        self.configured = True
        
    def build(self):
        command = self._get_command()
        host = self.gateway
        if self.remote:
            command = 'ssh %s "%s"'%(self.hostname, command)
            
        for pb_command in self.prebuild_commands:
            self._debug_print("Running:" + pb_command + '\n')
            ssh = subprocess.Popen(pb_command.split(),
                       shell=False,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
            self._debug_print(ssh.stdout.readlines() )          
            self._debug_print("Returned: " + str(ssh.returncode) + '\n')  
            ssh.terminate()
        
        if self.remote:
            command_list = ["ssh", "%s@%s" % (self.login,host), ' '.join(command)]
        else:
            command_list = command
            
        self._debug_print("Building: " + ' '.join(command_list) + ' from: ' + os.getcwd() + '\n')
        self.build_process = subprocess.Popen(' '.join(command_list),
                       shell=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
        if self.pid_queue:
            self.pid_queue.put(self.build_process.pid)
        
    def wait_until_done(self):
        self.build_process.wait()
        self.flush_messages()
        self._debug_print("Done building '%s' on %s."%(self.name, self.hostname) + '\n')
        return self.build_process.returncode
        
    def is_done(self):
        
        return self.build_process.poll() is not None
        
    def flush_messages(self):
        p = self.build_process
#        while p.poll() is None:
        
        for line in p.stdout.readlines():
            if self.stdout_cb:
                self.stdout_cb(line)
            
        for line in p.stderr.readlines():
            if self.stderr_cb:
                self.stderr_cb(line)
            
    def _debug_print(self, text):
        self.stdout_cb(text)
        return

    def _get_command(self):
        if not self.configured:
            raise exceptions.RuntimeError("Node not configured.")
        command = []
        if self.project_dir:
            command.append("cd %s;"%self.project_dir)
        command.append(self.build_command)
        if self.build_distributed_app:
            command.append('-s')
        command.append('-n')
        command.append(self.project_src)
        return command

class RemoteBuildNode(BuildNode):
    def __init__(self):
        super(RemoteBuildNode, self).__init__(hostname = 'gr01',
                 gateway = "nonce.mat.ucsb.edu",
                 login = 'sphere')
                 
        self.remote = True


class RunNode(Node):
    def __init__(self, name = 'node',
                    stdout_cb = lambda t: print(t, end=''), stderr_cb = lambda t: print(t, end='')):
        Node.__init__(self)
                    
        self.stdout_cb = stdout_cb
        self.stderr_cb = stderr_cb
        self.name = name
        
    def configure(self, run_dir = '',
                    path = '',
                    pid_queue = None):
        self.run_dir = run_dir
        self.path = path
        self.pid_queue = pid_queue

    def run(self):
        command = self.path
        
        self.stdout_cb("Running " + command + " from " + self.run_dir + " \n")
        
        try: 
            self.run_process = subprocess.Popen([command],
                           shell=False,
                           stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE,
                           cwd=self.run_dir)
            if self.pid_queue:
                self.pid_queue.put(self.run_process.pid)
            
#            if self.run_process.returncode:
#                return -1
            
            self.stdout_cb('PID %i\n'%(self.run_process.pid))

        except:
            import traceback
            self.stdout_cb( '\n'.join(traceback.format_tb(sys.exc_info()[2])))
        #p.join()
            #exit()
            
#        self.stdout_cb('Process ended.\n')
#        return p.returncode        
    
    def flush_messages(self):
        p = self.run_process
#        while p.poll() is None:
        for line in p.stdout.readlines():
            if self.stdout_cb:
                self.stdout_cb(line)
            
        for line in p.stderr.readlines():
            if self.stderr_cb:
                self.stderr_cb(line)

if __name__ == "__main__":
    builder = BuildNode('distributed app', False)
    # These tests assume you are running from an AlloProject directory
    # that contains an AlloSystem repo
    source = 'AlloSystem/alloutil/examples/allosphereApp.cpp'
    configuration = {"project_dir" : '',
                    "project_src": source,
                    "prebuild_commands" : '',
                    "build_command" : './run.sh '
                    }
    builder.configure(**configuration)
    builder.build()
    builder.wait_until_done()
    
    build_report = 'build/' + source[:-4].replace('/', '_') + '.json'
    import json
    with open(build_report) as fp:
        conf = json.load(fp)    
        for app in conf['apps']:
            runner = RunNode(app['type'] + '(local)')
            bin_path = conf['bin_dir'] + app['path']
            runner.configure(conf['root_dir'], bin_path)
            runner.run()
    print('Done.')