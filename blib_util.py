#!/usr/bin/env python
#-*- coding: ascii -*-

import os
try:
	import cfg_build
except:
	cfg_build_f = open("cfg_build.py", "w")
	cfg_build_f.write("""
################################################
# !!!! DO NOT DELETE ANY FIELD OF THIS FILE !!!!
################################################

compiler		= "auto"		# could be "vc12", vc11", "vc10", "vc9", "mingw", "gcc", "auto".
toolset			= "auto"		# could be "v120", "v120_xp", "v110", "v110_xp", "v100", "v90", "auto".
arch			= ("x86", )		# could be "x86", "x64", "arm_app", "x86_app"
config			= ("Debug", "RelWithDebInfo") # could be "Debug", "Release", "MinSizeRel", "RelWithDebInfo"
	""")
	cfg_build_f.close()
	import cfg_build

class compiler_info:
	def __init__(self, compiler, archs, cfg):
		import sys

		env = os.environ
		
		platform = sys.platform
		if 0 == platform.find("linux"):
			platform = "linux"

		try:
			cfg_build.compiler
		except:
			cfg_build.compiler = "auto"
		try:
			cfg_build.toolset
		except:
			cfg_build.toolset = "auto"
		try:
			cfg_build.arch
		except:
			cfg_build.arch = ("x86", )
		try:
			cfg_build.config
		except:
			cfg_build.config = ("Debug", "RelWithDebInfo")

		if "" == compiler:
			if ("" == cfg_build.compiler) or ("auto" == cfg_build.compiler):
				if "win32" == platform:
					if "VS120COMNTOOLS" in env:
						compiler = "vc12"
					elif "VS110COMNTOOLS" in env:
						compiler = "vc11"
					elif "VS100COMNTOOLS" in env:
						compiler = "vc10"
					elif "VS90COMNTOOLS" in env:
						compiler = "vc9"
					elif os.path.exists("C:\MinGW\bin\gcc.exe"):
						compiler = "mingw"
				elif "linux" == platform:
					compiler = "gcc"
				else:
					print("Unsupported platform\n")
					sys.exit(1)
			else:
				compiler = cfg_build.compiler

		toolset = cfg_build.toolset
		if ("" == cfg_build.toolset) or ("auto" == cfg_build.toolset):
			if "win32" == platform:
				if "vc12" == compiler:
					toolset = "v120"
				elif "vc11" == compiler:
					toolset = "v110"
				elif "vc10" == compiler:
					toolset = "v100"
				elif "vc9" == compiler:
					toolset = "v90"
				
		if "" == archs:
			archs = cfg_build.arch
			if "" == archs:
				archs = ("x86", )
				
		if "" == cfg:
			cfg = cfg_build.config
			if "" == cfg:
				cfg = ("Debug", "RelWithDebInfo")

		arch_list = []
		if "vc12" == compiler:
			compiler_name = "vc"
			compiler_version = 12
			for arch in archs:
				is_metro = False
				if (arch.find("_app") > 0):
					toolset = "v120"
					is_metro = True
				is_xp_toolset = False
				if (toolset.find("_xp") > 0):
					is_xp_toolset = True
				if ("x86" == arch) or ("x86_app" == arch):
					arch_list.append((arch, "Visual Studio 12", toolset, is_metro, is_xp_toolset))
				elif "arm_app" == arch:
					arch_list.append((arch, "Visual Studio 12 ARM", toolset, is_metro, is_xp_toolset))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 12 Win64", toolset, is_metro, is_xp_toolset))
		elif "vc11" == compiler:
			compiler_name = "vc"
			compiler_version = 11
			for arch in archs:
				is_metro = False
				if (arch.find("_app") > 0):
					toolset = "v110"
					is_metro = True
				is_xp_toolset = False
				if (toolset.find("_xp") > 0):
					is_xp_toolset = True
				if ("x86" == arch) or ("x86_app" == arch):
					arch_list.append((arch, "Visual Studio 11", toolset, is_metro, is_xp_toolset))
				elif "arm_app" == arch:
					arch_list.append((arch, "Visual Studio 11 ARM", toolset, is_metro, is_xp_toolset))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 11 Win64", toolset, is_metro, is_xp_toolset))
		elif "vc10" == compiler:
			compiler_name = "vc"
			compiler_version = 10
			for arch in archs:
				if "x86" == arch:
					arch_list.append((arch, "Visual Studio 10", toolset, False, False))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 10 Win64", toolset, False, False))
		elif "vc9" == compiler:
			compiler_name = "vc"
			compiler_version = 9
			for arch in archs:
				if "x86" == arch:
					arch_list.append((arch, "Visual Studio 9 2008", toolset, False, False))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 9 2008 Win64", toolset, False, False))
		elif "mingw" == compiler:
			compiler_name = "mgw"
			compiler_version = 0
			for arch in archs:
				arch_list.append((arch, "MinGW Makefiles", toolset, False, False))
		elif "gcc" == compiler:
			compiler_name = "gcc"
			compiler_version = 0
			for arch in archs:
				arch_list.append((arch, "Unix Makefiles", toolset, False, False))
		else:
			compiler_name = ""
			compiler_version = 0

		if "vc" == compiler_name:
			if compiler_version >= 10:
				self.use_msbuild = True
				self.proj_ext_name = "vcxproj"
			else:
				self.use_msbuild = False
				self.proj_ext_name = "vcproj"
		else:
			self.use_msbuild = False
			self.proj_ext_name = ""

		self.name = compiler_name
		self.version = compiler_version
		self.arch_list = arch_list
		self.cfg = cfg
		self.platform = platform
		
	def msvc_add_build_command(self, batch_cmd, sln_name, proj_name, config, arch = ""):
		if self.use_msbuild:
			batch_cmd.add_command('@SET VisualStudioVersion=%d.0' % self.version)
			if len(proj_name) != 0:
				file_name = "%s.%s" % (proj_name, self.proj_ext_name)
			else:
				file_name = "%s.sln" % sln_name
			config_str = "Configuration=%s" % config
			if len(arch) != 0:
				config_str = "%s,Platform=%s" % (config_str, arch)
			batch_cmd.add_command('MSBuild %s /m /v:m /p:%s' % (file_name, config_str))
		else:
			config_str = config
			if len(arch) != 0:
				config_str = "%s|%s" % (config_str, arch)
			proj_str = ""
			if len(proj_name) != 0:
				proj_str = "/project %s" % proj_name
			batch_cmd.add_command('devenv %s.sln /Build %s %s' % (sln_name, config_str, proj_str))

class batch_command:
	def __init__(self):
		self.commands_ = []

	def add_command(self, cmd):
		self.commands_ += [cmd]

	def execute(self):
		batch_file = "kge_build.bat"
		batch_f = open(batch_file, "w")
		batch_f.writelines([cmd_line + "\n" for cmd_line in self.commands_])
		batch_f.close()
		os.system(batch_file)
		os.remove(batch_file)
