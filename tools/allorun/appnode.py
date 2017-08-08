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
from time import sleep


from threading  import Thread
try:
    from Queue import Queue
except ImportError:
    from queue import Queue # python 3.x


ON_POSIX = 'posix' in sys.builtin_module_names

def enqueue_output(out, queue):
    for line in iter(out.readline, b''):
        queue.put(line)
    out.close()

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
        
        self.stdout = ''
        self.stderr = ''
        
        self.stdoutqueue = Queue()
        self.stderrqueue = Queue()
    
    def read_messages(self):
        self.flush_messages()
        std = self.stdout
        err = self.stderr
        self.stdout = ''
        self.stderr = ''
        return std, err
    
    def flush_messages(self):
        while not self.stdoutqueue.empty():
            line = self.stdoutqueue.get() # or q.get(timeout=.1)
            self.stdout += line
                
        while not self.stderrqueue.empty():
            line = self.stderrqueue.get() # or q.get(timeout=.1)
            self.stderr += line
                
    def is_done(self):
        if self.internal_process:
            return self.internal_process.poll() is not None
        else:
            return True

    def wait_until_done(self):
        if not self.internal_process:
            return 0
        self.internal_process.wait()
        self.flush_messages()
        #self._debug_print("Done building '%s' on %s."%(self.name, self.hostname) + '\n')
        return self.internal_process.returncode        
        
    def terminate(self):
        if self.internal_process and not self.is_done():
            self.internal_process.kill()
    
    def _debug_print(self, text):
        self.stdout += text
        return
    
class BuildNode(Node):
    def __init__(self, name = 'b_node', distributed = True):
        Node.__init__(self)

        self.name = name
        self.build_distributed_app = distributed
        
    def configure(self, **kwargs):
        self.project_dir = kwargs["project_dir"]
        self.project_src = kwargs["project_src"]
        self.prebuild_commands = kwargs["prebuild_commands"]
        self.build_command = kwargs["build_command"]
        self.configured = True
        
    def build(self):
        # Execute pre-build commands
        for pb_command in self.prebuild_commands:
            self._debug_print("-- Running:" + pb_command + '\n')
            ssh = subprocess.Popen(pb_command.split(),
                       shell=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
            self._debug_print(ssh.stdout.readlines() )          
            self._debug_print("Returned: " + str(ssh.returncode) + '\n')  
            ssh.terminate()
        
        command_list = self._get_build_command()
        if self.remote:
            command_list = ['ssh', '-o StrictHostKeyChecking=no', self.hostname, '"%s"'%' '.join(command_list)]
            if self.gateway != '' :
                command_list = ["ssh", "-o StrictHostKeyChecking=no",
                                "%s@%s" % (self.login, self.gateway), ' '.join(command_list)]

            
        self._debug_print("-- Building: " + ' '.join(command_list) + '\n')
        self.internal_process = subprocess.Popen(' '.join(command_list),
                       shell=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
        if self.internal_process.pid <= 0:
            self._debug_print("Process failed to start.")
        t = Thread(target=enqueue_output, args=(self.internal_process.stdout, self.stdoutqueue))
        t.daemon = True # thread dies with the program
        t.start()
        terr = Thread(target=enqueue_output, args=(self.internal_process.stderr, self.stderrqueue))
        terr.daemon = True # thread dies with the program
        terr.start()

    def _get_build_command(self):
        if not self.configured:
            raise exceptions.RuntimeError("Node not configured.")
        command = []
        if self.project_dir:
            command.append("cd %s;"%self.project_dir)
        command.append(self.build_command)
#        if self.build_distributed_app:
#            command.append('-s')
#        command.append('-n')
        command.append(self.project_src)
        return command

class RemoteBuildNode(BuildNode):
    def __init__(self, hostname = 'gr01',
                 gateway = "nonce.mat.ucsb.edu",
                 login = 'sphere',
                 deploy_to = []):
        BuildNode.__init__(self)
        self.hostname = hostname
        self.name = hostname
        self.gateway = gateway
        self.login = login
        self.deploy_to = deploy_to
        
        self.remote = True

class RunNode(Node):
    def __init__(self, name = 'node'):
        Node.__init__(self)

        self.name = name
        
    def configure(self, run_dir = '',
                    path = ''):
        self.run_dir = run_dir
        self.path = path

    def run(self):
        
        command = self.path
        if self.remote:
            command_list = ["ssh", "%s@%s" % (self.login,self.gateway), command]
        else:
            command_list = [command]
        
        self._debug_print("-- Running " + ' '.join(command_list) + " from " + self.run_dir + " \n")
        
        try: 
            self.internal_process = subprocess.Popen(command_list,
                           shell=False,
                           stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE,
                           cwd=self.run_dir)
            t = Thread(target=enqueue_output, args=(self.internal_process.stdout, self.stdoutqueue))
            t.daemon = True # thread dies with the program
            t.start()
            t = Thread(target=enqueue_output, args=(self.internal_process.stderr, self.stderrqueue))
            t.daemon = True # thread dies with the program
            t.start()

        except:
            import traceback
            self._debug_print( '\n'.join(traceback.format_tb(sys.exc_info()[2])))


if __name__ == "__main__":
    builder = BuildNode('distributed app', False)
    # These tests assume you are running from an AlloProject directory
    # that contains an AlloSystem repo
    source = 'AlloSystem/alloutil/examples/allosphereApp.cpp'
    configuration = {"project_dir" : '',
                    "project_src": source,
                    "prebuild_commands" : [],
                    "build_command" : './run.sh '
                    }
    builder.configure(**configuration)
    builder.build()
    
    while not builder.is_done():
        std, err = builder.read_messages()
        if std != '':
            print(std)
        sleep(1)
    
    build_report = 'build/' + source[:-4].replace('/', '_') + '.json'
    import json
    with open(build_report) as fp:
        conf = json.load(fp)    
        for app in conf['apps']:
            runner = RunNode(app['type'] + '(local)')
            bin_path = conf['bin_dir'] + app['path']
            runner.configure(conf['root_dir'], bin_path)
            runner.run()
            while not runner.is_done():
                std, err = runner.read_messages()
                if std != '':
                    print(std)
                sleep(1)
    print('Done.')
