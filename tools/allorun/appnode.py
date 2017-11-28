# -*- coding: utf-8 -*-
"""
Created on Wed Mar 30 21:28:32 2016

@author: andres
"""


from __future__ import print_function

#import exceptions
import subprocess
from subprocess import TimeoutExpired
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

def which(program):
    import os
    def is_exe(fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file

    return None


def build_thread_func(builder):
    command_lists = builder._get_build_commands()
    if builder.remote:
        for i in range(len(command_lists)):
            command_lists[i] = ['ssh', '-o StrictHostKeyChecking=no', "%s@%s" % (builder.login, builder.hostname), ' eval "%s"'%' '.join(command_lists[i])]
    else:
        os.chdir(builder.project_dir);
#    builder._debug_print(str(command_lists))
    for command in command_lists:
        builder._debug_print("-- Building: " + ' '.join(command) + " from " + os.getcwd() + '\n')
        builder.internal_process = subprocess.Popen(' '.join(command),
                       shell=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
        if builder.internal_process.pid <= 0:
            builder._debug_print("Process failed to start.")
        else:
            builder._debug_print("Process started normally")
        while builder.internal_process.returncode is None:
            try:
                outs, errs = builder.internal_process.communicate(timeout=0.01)
                builder._debug_print(outs.decode("utf-8"))
                builder._debug_print('EE ' + errs.decode("utf-8"))
            except TimeoutExpired as e:
                for line in iter(builder.internal_process.stdout.readline, b''):
                    builder._debug_print(line.decode('utf-8'))
                builder.internal_process.stdout.close()
                for line in iter(builder.internal_process.stderr.readline, b''):
                    builder._debug_print('EE ' + line.decode('utf-8'))
                builder.internal_process.stderr.close()
                pass

            builder.internal_process.poll()
    builder._debug_print("Builder Thread done")

def run_thread_func(runner, run_command_lists):

    for run_command in run_command_lists:
        if runner.remote:
            run_command = "ssh %s@%s eval 'export DISPLAY=:0;cd %s;%s'"%(runner.login, runner.hostname, runner.run_dir, run_command)
        else:
            os.chdir(runner.run_dir);

        runner._debug_print("---. Running: " + run_command + " from " + runner.run_dir + '\n')
        runner.internal_process = subprocess.Popen(run_command,
                       shell=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
        if runner.internal_process.pid <= 0:
            runner._debug_print("Process failed to start.")
        else:
            runner._debug_print("Process started normally")
        while runner.internal_process.returncode is None:
            try:
                outs, errs = runner.internal_process.communicate(timeout=0.01)
                runner._debug_print(outs.decode("utf-8"))
                runner._debug_print('EE ' + errs.decode("utf-8"))
            except TimeoutExpired as e:
                for line in iter(runner.internal_process.stdout.readline, b''):
                    runner._debug_print(line.decode('utf-8'))
                runner.internal_process.stdout.close()
                for line in iter(runner.internal_process.stderr.readline, b''):
                    runner._debug_print('EE ' + line.decode('utf-8'))
                runner.internal_process.stderr.close()
                pass

            runner.internal_process.poll()
    runner._debug_print("Runner Thread done")

class Node:
    def __init__(self,
                 hostname = 'localhost',
                 login = '',
                 ):
        self.remote = False
        self.hostname = hostname
        self.login = login
        self._next_node = None
        self.internal_process = None
        self.thread = None
        self.name = None

        self.stdout = ''
        self.stderr = ''

    def read_messages(self):
        std = self.stdout
        err = self.stderr
        self.stdout = ''
        self.stderr = ''
        return std, err

    def is_done(self):
        if self.thread:
            self.thread.join(0.01)
            return not self.thread.is_alive()
        else:
            return False

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

    def next_node(self, next_node):
        self._next_node = next_node

    def get_next_node(self):
        return self._next_node

    def _debug_print(self, text):
        if not text == '':
            if not text[-1] == '\n':
                text += '\n'
            self.stdout += '[%s] '%self.name + str(text)

class BuildNode(Node):
    def __init__(self, name = 'b_node', distributed = True):
        Node.__init__(self)

        self.name = name
        self.build_distributed_app = distributed
        self.prebuild_commands = []
        self.debug = False
        self.cmake = "cmake"
        self.deploy_to = []
        self.scratch_path = '/alloshare/scratch'

    def configure(self, **kwargs):
        if "project_dir" in kwargs:
            self.project_dir = kwargs["project_dir"]
        if "project_src" in kwargs:
            self.project_src = kwargs["project_src"]
            if len(self.deploy_to) == 0:
                self.deploy_to = [["localhost"] for i in range(len(self.project_src)) ]
        if "prebuild_commands" in kwargs:
            self.prebuild_commands = kwargs["prebuild_commands"]
        if "build_commands" in kwargs:
            self.build_commands = kwargs["build_commands"]
        if "cmake" in kwargs:
            self.cmake = kwargs["cmake"]
        if "scratch_path" in kwargs:
            self.scratch_path = kwargs["scratch_path"]
        self.configured = True

    def set_debug(self, debug_ = True):
        self.debug = debug_

    def build(self):
        # Execute pre-build commands
        for pb_command in self.prebuild_commands:
            self._debug_print("-- Running (pre-build):" + pb_command + '\n')
            ssh = subprocess.Popen(pb_command.split(),
                       shell=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
            self._debug_print(ssh.stdout.readlines() )
            self._debug_print("Returned: " + str(ssh.returncode) + '\n')
            ssh.terminate()

        self.thread = Thread(target=build_thread_func, args=[self])
        self.thread.daemon = True # thread dies with the program
        self.thread.start()

    def _get_build_commands(self):
        use_ninja = False
        if not self.remote and which("ninja"): use_ninja = True
        if not self.configured:
            raise exceptions.RuntimeError("Node not configured.")
        commands = []
        if not type(self.project_src) == list:
            self.project_src = [self.project_src]
        for build_comm in self.build_commands:
            if build_comm == "$$cmake":
                generator = ''
                if sys.platform == 'win32':
                    generator = '-GMSYS Makefiles'
                elif use_ninja:
                    generator = '-GNinja'

                debug_flags = ['-DRUN_IN_DEBUGGER=0']
                build_debug = self.debug
                if build_debug:
                    debugger = "gdb"
                    debug_flags = ['-DRUN_IN_DEBUGGER=1',
                                   "-DALLOSYSTEM_DEBUGGER=%s"%debugger,
                                   '-DCMAKE_BUILD_TYPE=Debug']

                command = []
                src = ';'.join(self.project_src)
                if self.project_dir:
                    command.append("cd %s;"%self.project_dir)
                if os.path.isdir(src):
                    if self.remote:
                        target_flag = '-DALLOPROJECT_BUILD_APP_DIR=\\"%s\\"'%src
                    else:
                        target_flag = '-DALLOPROJECT_BUILD_APP_DIR="%s"'%src
                    build_flag = "-DALLOPROJECT_BUILD_DIR=1"
                else:
                    if self.remote:
                        target_flag = '-DALLOPROJECT_BUILD_APP_FILE=\\"%s\\"'%src
                    else:
                        target_flag = '-DALLOPROJECT_BUILD_APP_FILE="%s"'%src
                    build_flag = "-DALLOPROJECT_BUILD_DIR=0"
                command += [self.cmake, '.',  generator, target_flag, build_flag]
                command += debug_flags

                commands.append(command)
            elif build_comm == "$$make":
                command = []
                if self.project_dir:
                    command.append("cd %s;"%self.project_dir)
                targets = self.get_products()
                if use_ninja:
                    command += ['ninja'] + targets
                else:
                    command += ['make'] + targets + ['-j7']
                commands.append(command)
            else:
                for src in self.project_src:
                    command.append(build_comm)
    #        if self.build_distributed_app:
    #            command.append('-s')
    #        command.append('-n')
                    command.append(src)
                    commands.append(command)
        return commands

    def get_products(self):
        products = [os.path.splitext(src)[0].replace('/', '_') for src in self.project_src]
        return products

class RemoteBuildNode(BuildNode):
    def __init__(self, hostname = 'gr01',
                 login = 'sphere',
                 deploy_to = []):
        BuildNode.__init__(self)
        self.hostname = hostname
        self.name = hostname
        self.login = login
        self.deploy_to = deploy_to

        self.remote = True

class RunNode(Node):
    def __init__(self, name = 'node'):
        Node.__init__(self)

        self.name = name

    def configure(self,
                  run_dir = '',
                  path = ''):
        self.run_dir = run_dir
        self.path = path

    def run(self):

        command = self.path
        if not type(command) == list:
            command_list = [command]
        else:
            command_list = command

        self.thread = Thread(target=run_thread_func, args=(self, command_list))
        self.thread.daemon = True # thread dies with the program
        self.thread.start()

class RemoteRunNode(RunNode):
    def __init__(self, name, hostname = 'gr01',
                 login = 'sphere'):
        RunNode.__init__(self, name)
        self.hostname = hostname
        self.login = login

        self.remote = True

if __name__ == "__main__":
    builder = BuildNode('distributed app', False)
    # These tests assume you are running from an AlloProject directory
    # that contains an AlloSystem repo
    source = 'AlloSystem/alloutil/examples/allosphereApp.cpp'
    configuration = {"project_dir" : '',
                    "project_src": source,
                    "prebuild_commands" : [],
                    "build_commands" : ['./run.sh ']
                    }
    builder.configure(**configuration)
    builder.build()

    while not builder.is_done():
        std, err = builder.read_messages()
        if std != '':
            print(std)
        sleep(1)

#    build_report = 'build/' + source[:-4].replace('/', '_') + '.json'
#    import json
#    with open(build_report) as fp:
#        conf = json.load(fp)
#        for app in conf['apps']:
#            runner = RunNode(app['type'] + '(local)')
#            bin_path = conf['bin_dir'] + app['path']
#            runner.configure(conf['root_dir'], bin_path)
#            runner.run()
#            while not runner.is_done():
#                std, err = runner.read_messages()
#                if std != '':
#                    print(std)
#                sleep(1)
    print('Done.')
