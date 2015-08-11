#!/usr/bin/env python
# -*- coding: utf-8 -*-

# TODO decide how to bring in bugfixes from the alloproject scripts
# TODO figure out how to align the Dependency checkboxes!

import os
import platform
import stat
import urllib2
from subprocess import Popen, PIPE

import Tkinter as tk
import tkFileDialog


class Project():
    def __init__(self):
        self.use_installed_libs = False
        self.dependencies = {}
        self.dependencies['GLV'] = True
        self.dependencies['Cuttlebone'] = True
        self.dependencies['Gamma'] = True
        self.dir = ''
        self.project_name = ''
        
    def make_project_folder(self, folder, name):
        fullpath = folder + '/' + name
        if os.path.isdir(fullpath):
            return False
        os.mkdir(fullpath)
        if not os.path.isdir(fullpath):
            return False
        self.dir = folder
        self.project_name = name
        
        src_files = ['common.hpp','simpleApp.cpp', 'flags.cmake']
        curpath = os.getcwd()
        os.mkdir(fullpath + '/src')
        os.chdir(fullpath + '/src')
        for src_file in src_files:
            response = urllib2.urlopen('https://raw.githubusercontent.com/AlloSphere-Research-Group/AlloProject/master/src/%s'%src_file)
            f = open(fullpath + '/src/' + src_file, 'w')
            f.write(response.read())
        os.chdir(curpath)
        return True
        
    def make_init_script(self):
        script_text = '#!/bin/sh\n\n'
        files_to_get = ['CMakeLists.txt', 'run.sh', 'distclean']
        if platform.system() == "Darwin":
            files_to_get.append('makexcode.sh')
        for file_to_get in files_to_get:
            script_text += 'wget -N https://raw.githubusercontent.com/AlloSphere-Research-Group/AlloProject/master/%s\n'%file_to_get

        script_text += '\nchmod 750 run.sh\nchmod 750 distclean\n\n'
        cmake_modules = ['make_dep.cmake', 'CMakeRunTargets.cmake']
        script_text += 'mkdir -p cmake_modules\ncd cmake_modules\n'

        for module in cmake_modules:
            script_text += 'wget -N https://raw.githubusercontent.com/AlloSphere-Research-Group/AlloProject/master/cmake_modules/%s\n'%module
        
        script_text += '\ncd ..\n'
        if not self.use_installed_libs:
            script_text += 'git clone -b devel --depth 1 https://github.com/AlloSphere-Research-Group/AlloSystem.git AlloSystem\n'
            if self.dependencies['Cuttlebone']:
                script_text += 'git clone --depth 1 https://github.com/rbtsx/cuttlebone.git cuttlebone\n'
            if self.dependencies['Gamma']:
                script_text += 'git clone --depth 1 https://github.com/AlloSphere-Research-Group/Gamma.git Gamma\n'
            if self.dependencies['GLV']:
                script_text += 'git clone --depth 1 https://github.com/AlloSphere-Research-Group/GLV.git GLV\n'
        
        script_text += '''git fat init
git fat pull
'''
        
        curpath = os.getcwd()
        os.chdir(self.dir + '/' + self.project_name)
        f = open('initproject.sh', 'w')
        f.write(script_text)
        os.chmod('initproject.sh', stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR | stat.S_IRGRP | stat.S_IWGRP | stat.S_IXGRP)
        os.chdir(curpath)        
        
    def run_init_script(self):
        curpath = os.getcwd()
        os.chdir(self.dir + '/' + self.project_name)
        p = Popen(['./initproject.sh'], stdout=PIPE, stderr=PIPE, stdin=PIPE)
        stdout = []
        while True:
            line = p.stdout.readline()
            stdout.append(line)
            print line,
            if line == '' and p.poll() != None:
                break
        output = '\n'.join(stdout)
        os.chdir(curpath)  
        return output
    

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.grid(padx=10, pady=10, ipadx=5, ipady=10)
        self.project = Project()
        self.createWidgets()
        
    def createWidgets(self):
        self.name_label = tk.Label(self)
        self.name_label["text"] = "Project Name:"
        self.name_label.grid(row=0, column=0)
        
        self.project_name = tk.Entry(self, width = 40)
        self.project_name.grid(row=0, column=1)
        
        self.dir_label = tk.Label(self)
        self.dir_label["text"] = "Project Directory:"
        self.dir_label.grid(row=1, column=0)
        
        self.project_dir = tk.Entry(self, width = 40)
        self.project_dir.grid(row=1, column=1)
        self.project_dir.insert(0, os.getcwd())
        self.project_dir_select = tk.Button(self, text="...",
                                            command=self.select_directory)
        self.project_dir_select.grid(row=1, column=2)
        
        self.libs_button_value = tk.IntVar()
        self.use_installed_libs = tk.Radiobutton(self,
                                                 text='Use installed libs',
                                                 variable=self.libs_button_value,
                                                 value=1,
                                                 command=self.set_libs_status)
        self.use_installed_libs.grid(row=5, column=0)
        self.get_libs = tk.Radiobutton(self,
                                       text='Get libs',
                                       variable=self.libs_button_value,
                                       value = 0,
                                       command=self.set_libs_status)
        self.get_libs.grid(row=6, column=0)
        
        self.lib_label = tk.Label(self, text= "Additional libraries:")
        self.lib_label.grid(row=10, column=0)
        
        self.libs = []  
        self.glv = tk.Checkbutton(self, text='GLV', anchor=tk.NW, justify=tk.LEFT)
        if self.project.dependencies['GLV']:       
            self.glv.select()
        else:
            self.glv.deselect()
        self.glv.grid(row=11, column=0)
        self.libs.append(self.glv)
        
        self.gamma = tk.Checkbutton(self, text='Gamma', anchor=tk.NW, justify=tk.LEFT)
        if self.project.dependencies['Gamma']:       
            self.gamma.select()
        else:
            self.gamma.deselect()
        self.gamma.grid(row=12, column=0)
        self.libs.append(self.gamma)
        
        self.cuttlebone = tk.Checkbutton(self, text='Cuttlebone', anchor=tk.NW, justify=tk.LEFT)
        if self.project.dependencies['Cuttlebone']:       
            self.cuttlebone.select()
        else:
            self.cuttlebone.deselect()    
        self.cuttlebone.grid(row=13, column=0)
        self.libs.append(self.cuttlebone)
        
        self.Accept = tk.Button(self, text="Make project", 
                                command=self.make_project,
                                width=60)
        self.Accept.grid(row=50, column=0, columnspan=3)
        
        self.console = tk.Text(self, height = 10)
        self.console.grid(row=60, column=0, columnspan=3, pady=10)
  
    def select_directory(self):
        project_dir = self.location_dialog = tkFileDialog.askdirectory(parent=root,
                                                                       initialdir=self.project_dir.get(),
                                                                       title='Please select a directory')
        #print "dir = ", project_dir
        if project_dir:
            self.project_dir.delete(0, tk.END)
            self.project_dir.insert(0, project_dir)
            
    def set_libs_status(self):
        if self.libs_button_value.get() == 0:
            self.project.use_installed_libs = False
            self.lib_label.grid()
            for lib in self.libs:
                lib.grid()
            
        elif self.libs_button_value.get() == 1:
            self.project.use_installed_libs = True
            self.lib_label.grid_remove()
            for lib in self.libs:
                lib.grid_remove()
        
    def make_project(self):
        name = self.project_name.get()
        folder = self.project_dir.get()
        if not name:
              wdw = tk.Toplevel()
              #wdw.geometry('+400+400')
              e = tk.Label(wdw, text='Please choose a project name')
              e.pack(ipadx=50, ipady=10, pady=10)
              b = tk.Button(wdw, text='OK', command=wdw.destroy)
              b.pack(ipadx=50, pady=10)
              wdw.transient(root)
              wdw.grab_set()
              root.wait_window(wdw)
              return
        self.console.insert(tk.END, 'Creating project -------\n')
        ok = self.project.make_project_folder(folder, name)
        if not ok:
            wdw = tk.Toplevel()
            #wdw.geometry('+400+400')
            e = tk.Label(wdw, text="Can't create project folder.")
            e.pack(ipadx=50, ipady=10, pady=10)
            b = tk.Button(wdw, text='OK', command=wdw.destroy)
            b.pack(ipadx=50, pady=10)
            wdw.transient(root)
            wdw.grab_set()
            root.wait_window(wdw)
            self.console.insert(tk.END, "Couldn't create folder\n")
            return
        self.project.make_init_script() 
        self.update_idletasks()
        self.console.insert(tk.END, 'Running init_project.sh -------\n')
        self.update_idletasks()
        output = self.project.run_init_script()
        self.console.insert(tk.END, output + '\n')
        self.console.insert(tk.END, 'Project created succesfully.\n')


root = tk.Tk()
root.title('AlloProject')
app = Application(master=root)
app.mainloop()




