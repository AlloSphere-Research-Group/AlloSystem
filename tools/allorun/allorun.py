#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import sys, os
import argparse

sys.path.append("AlloSystem/tools/allorun/")
from appnode import BuildNode, RemoteBuildNode, RunNode, RemoteRunNode

class AlloRunner():
    def __init__(self, nodes, source, verbose = True, debug = False):
        self.current_index = 0
        self.buffer_len = 256
        self.chase = True
        self.verbose = verbose
        self.debug = debug

        self.log_text = ''
        if self.verbose:
            self.log('Started...\n')
        if nodes and len(nodes) > 0:
            self.nodes = nodes
            if self.verbose:
                for node in nodes:
                    self.log("----- Node -----")
                    self.log("Remote: " + str(node.remote))
                    self.log("Hostname:" + node.hostname)

        #self.item_color = { "none": 7, "error" : 1, "done" : 2, "working" : 4}

        self.displays = ["STDOUT", "STDERR"]
        self.current_display = 0
        self.source = source
        pass

    def log(self, text):
        if not text == '':
            if not text[-1] == '\n':
                text = text + '\n'
            self.log_text += text
            print(text, end = '')

    def start(self):
        if (self._run_builders()) :
            self._start_runners()
        else:
            self.log("Building aborted.")

    def _run_builders(self):
        self.builders = []
        self.stdoutbuf  = ['' for i in self.nodes]
        self.stderrbuf = ['' for i in self.nodes]
        for i, node in enumerate(self.nodes):

            builder = node
            from os.path import expanduser
            home = expanduser("~")
            cwd = os.getcwd()
            if builder.remote:
                cwd = base_path + '/' + user_prefix + '/' + node.hostname + '/' + cwd[cwd.rfind('/')+ 1 :]
#                if cwd[:len(home)] == home:
#                    cwd = cwd[len(home) + 1:]
                print(cwd)

            if self.source:
                configuration = {"project_dir" : cwd,
                                 "project_src": self.source}
            else:
                configuration = {"project_dir" : cwd }
            builder.configure(**configuration)
            builder.set_debug(self.debug)
            self.builders.append(builder)
            builder.build()

        done = True
        self.log("Start build")
        for b in self.builders:
            if not b.is_done():
                done = False
                break
        while not done:
            for i in range(len(self.builders)):
                std, err = self.builders[i].read_messages()
                self.stdoutbuf[i] += std
                self.stderrbuf[i] += err

                if std:
                    self.log(std)
                if err:
                    self.log(err)

#                num_lines = console_output.count('\n') + 1
#                if num_lines >= self.buffer_len:
#                    lines = console_output.split('\n')
#                    console_output = '\n'.join(lines[len(lines) - self.buffer_len:])
#
#                    num_lines = console_output.count('\n')

            done = True
            for b in self.builders:
                if not b.is_done():
                    done = False
                    break

        self.log(builder.stdout)
        self.log(builder.stderr)
        self.log("Building done  ---- ")

        return True

    def _start_runners(self):
        self.stdoutbuf  = []
        self.stderrbuf = []
        self.runners = []
        for i, node in enumerate(self.nodes):
            for src, app, deploy_hosts in zip(node.project_src, node.get_products(), node.deploy_to):
                if node.remote:
                    for deploy_host in deploy_hosts:
                        self.stdoutbuf.append('')
                        self.stderrbuf.append('')
                        runner = RemoteRunNode(deploy_host + '-' + app, deploy_host)
                        bin_path = 'build/bin/' + app
                        runner.configure(node.project_dir, bin_path)
                        runner.run()
                        self.runners.append(runner)
                else:
                    build_report = 'build/' + src[:-4].replace('/', '_') + '.json'
                    import json
                    if self.verbose:
                        self.log("Reading build report:" + build_report)
                    with open(build_report) as fp:
                        conf = json.load(fp)
                        for app in conf['apps']:
                            if self.verbose:
                                self.log("New runner    :" + app['type'])
                                self.log("       bin dir:" + conf['bin_dir'])
                                self.log("          path:" + app['path'])
                                self.log("      root_dir:" + conf['root_dir'])
#                            if type(node) == RemoteBuildNode:
#                                self.runstdoutbuf.append(str(node.deploy_to))

                            self.stdoutbuf.append('')
                            self.stderrbuf.append('')
                            runner = RunNode('local-' + app['path'], )
                            bin_path = conf['bin_dir'] + app['path']
                            runner.configure(conf['root_dir'], bin_path)
                            runner.run()
                            self.runners.append(runner)
        done = False

        while not done:
            for i in range(len(self.runners)):
                std, err = self.runners[i].read_messages()
                self.stdoutbuf[i] += std
                self.stderrbuf[i] += err

                if std:
                    self.log(std)
                if err:
                    self.log(err)

            done = True
            for b in self.runners:
                if not b.is_done():
                    done = False
                    break

        for runner in self.runners:
            self.log(runner.stdout)
            self.log(runner.stderr)
            runner.thread.join(0.1)

        self.log("Running done.")


    def stop(self):
        for runner in self.runners:
            runner.terminate()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Build and Run AlloSphere apps.')
    parser.add_argument('source', type=str, nargs="*",
                       help='source file or directory to build',
                       default='')
    parser.add_argument('-l', '--local', help='Force local build.', action="store_true")
    parser.add_argument('-a', '--allosphere', help='Force Allosphere build.', action="store_true")
    parser.add_argument('-v', '--verbose', help='Verbose output.', action="store_true")
    parser.add_argument('-d', '--debug', help='Build debug version.', action="store_true")

    args = parser.parse_args()
    project_src = args.source

    import socket
    hostname = socket.gethostname()

    cur_dir = os.getcwd()
    project_name = cur_dir[cur_dir.rindex('/') + 1:]


    if (hostname == 'audio.10g' and not args.local) or (args.allosphere):
        from allosphere import nodes
    else:
        from local import nodes

    if args.allosphere:
        base_path = '/alloshare/scratch'
        user_prefix = 'andres'
        for node in nodes:
            exclude_dirs = ['"CMakeCache.txt"', '"build"', '".git"', '"AlloSystem/.git"', '"AlloSystem/CMakeFiles"','"*/CMakeFiles"', '"*/AlloSystem-build"']
            rsync_command = 'rsync -arv -p --delete --group=users ' + "--exclude " + ' --exclude '.join(exclude_dirs) + " "
            rsync_command += cur_dir + ' '
            rsync_command += "sphere@%s:"%node.hostname + base_path + "/" + user_prefix + "/" + node.hostname
            print("syncing:  " + rsync_command)
            os.system(rsync_command)

    import traceback

    if len(nodes) > 0:
        app = AlloRunner(nodes, project_src, args.verbose, args.debug)
        try:
            app.start()
        except:
            print("Python exception ---")
            traceback.print_exc()
            print(sys.exc_info()[0])
#        if not app.log_text == '':
#            print('Output log: -------------\n' + app.log_text)
    else:
        print("No nodes. Aborting.")


