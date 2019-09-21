#!/usr/bin/python
# ----------------------------------------------------------------------------
# cocos2d "compile" plugin
#
# Copyright 2013 (C) Luis Parravicini
#
# License: MIT
# ----------------------------------------------------------------------------
'''
"compile" plugin for cocos command line tool
'''

__docformat__ = 'restructuredtext'

import multiprocessing
import cocos
from MultiLanguage import MultiLanguage
import cocos_project
import os
import re
import sys
import shutil
import json
import build_web
import utils

class CCPluginCompile(cocos.CCPlugin):
    """
    compiles a project
    """

    BUILD_CONFIG_FILE = "build-cfg.json"
    CFG_KEY_WIN32_COPY_FILES = "copy_files"
    CFG_KEY_WIN32_MUST_COPY_FILES = "must_copy_files"

    CFG_KEY_COPY_RESOURCES = "copy_resources"
    CFG_KEY_MUST_COPY_RESOURCES = "must_copy_resources"

    OUTPUT_DIR_NATIVE = "bin"
    OUTPUT_DIR_SCRIPT_DEBUG = "simulator"
    OUTPUT_DIR_SCRIPT_RELEASE = "publish"
    WEB_PLATFORM_FOLDER_NAME = "html5"

    PROJ_CFG_KEY_IOS_SIGN_ID = "ios_sign_id"
    PROJ_CFG_KEY_ENGINE_DIR = "engine_dir"

    BACKUP_SUFFIX = "-backup"
    ENGINE_JS_DIRS = [
        "frameworks/js-bindings/bindings/script",
        "cocos/scripting/js-bindings/script"
    ]

    @staticmethod
    def plugin_name():
        return "compile"

    @staticmethod
    def brief_description():
        return MultiLanguage.get_string('COMPILE_BRIEF')

    def _add_custom_options(self, parser):
        from argparse import ArgumentParser
        parser.add_argument("-m", "--mode", dest="mode", default='debug',
                          help=MultiLanguage.get_string('COMPILE_ARG_MODE'))
        parser.add_argument("-j", "--jobs", dest="jobs", type=int,
                          help=MultiLanguage.get_string('COMPILE_ARG_JOBS'))
        parser.add_argument("-o", "--output-dir", dest="output_dir",
                            help=MultiLanguage.get_string('COMPILE_ARG_OUTPUT'))

        group = parser.add_argument_group(MultiLanguage.get_string('COMPILE_ARG_GROUP_ANDROID'))
        group.add_argument("--ap", dest="android_platform",
                           help=MultiLanguage.get_string('COMPILE_ARG_AP'))
        group.add_argument("--build-type", dest="build_type",
                           help=MultiLanguage.get_string('COMPILE_ARG_BUILD_TYPE'))
        group.add_argument("--app-abi", dest="app_abi",
                           help=MultiLanguage.get_string('COMPILE_ARG_APP_ABI'))
        group.add_argument("--ndk-toolchain", dest="toolchain",
                           help=MultiLanguage.get_string('COMPILE_ARG_TOOLCHAIN'))
        group.add_argument("--ndk-cppflags", dest="cppflags",
                           help=MultiLanguage.get_string('COMPILE_ARG_CPPFLAGS'))
        group.add_argument("--no-apk", dest="no_apk", action="store_true",
                           help=MultiLanguage.get_string('COMPILE_ARG_NO_APK'))
        group.add_argument("--no-sign", dest="no_sign", action="store_true",
                           help=MultiLanguage.get_string('COMPILE_ARG_NO_SIGN'))

        group = parser.add_argument_group(MultiLanguage.get_string('COMPILE_ARG_GROUP_WIN'))
        group.add_argument("--vs", dest="vs_version", type=int,
                           help=MultiLanguage.get_string('COMPILE_ARG_VS'))

        group = parser.add_argument_group(MultiLanguage.get_string('COMPILE_ARG_GROUP_WEB'))
        group.add_argument("--source-map", dest="source_map", action="store_true",
                           help=MultiLanguage.get_string('COMPILE_ARG_SOURCE_MAP'))
        group.add_argument("--advanced", dest="advanced", action="store_true",
                           help=MultiLanguage.get_string('COMPILE_ARG_ADVANCE'))

        group = parser.add_argument_group(MultiLanguage.get_string('COMPILE_ARG_GROUP_IOS_MAC'))
        group.add_argument("-t", "--target", dest="target_name",
                           help=MultiLanguage.get_string('COMPILE_ARG_TARGET'))

        group = parser.add_argument_group(MultiLanguage.get_string('COMPILE_ARG_GROUP_IOS'))
        group.add_argument("--sign-identity", dest="sign_id",
                           help=MultiLanguage.get_string('COMPILE_ARG_IOS_SIGN'))

        group = parser.add_argument_group(MultiLanguage.get_string('COMPILE_ARG_GROUP_LUA_JS'))
        group.add_argument("--no-res", dest="no_res", action="store_true",
                           help=MultiLanguage.get_string('COMPILE_ARG_NO_RES'))
        group.add_argument("--compile-script", dest="compile_script", type=int, choices=[0, 1],
                           help=MultiLanguage.get_string('COMPILE_ARG_COMPILE_SCRIPT'))

        group = parser.add_argument_group(MultiLanguage.get_string('COMPILE_ARG_GROUP_LUA'))
        group.add_argument("--lua-encrypt", dest="lua_encrypt", action="store_true",
                           help=MultiLanguage.get_string('COMPILE_ARG_LUA_ENCRYPT'))
        group.add_argument("--lua-encrypt-key", dest="lua_encrypt_key",
                           help=MultiLanguage.get_string('COMPILE_ARG_LUA_ENCRYPT_KEY'))
        group.add_argument("--lua-encrypt-sign", dest="lua_encrypt_sign",
                           help=MultiLanguage.get_string('COMPILE_ARG_LUA_ENCRYPT_SIGN'))

        category = self.plugin_category()
        name = self.plugin_name()
        usage = "\n\t%%prog %s %s -p <platform> [-s src_dir][-m <debug|release>]" \
                "\nSample:" \
                "\n\t%%prog %s %s -p android" % (category, name, category, name)

    def _check_custom_options(self, args):
        # get the mode parameter
        available_modes = [ 'release', 'debug' ]
        self._mode = self.check_param(args.mode, 'debug', available_modes,
                                      MultiLanguage.get_string('COMPILE_ERROR_WRONG_MODE_FMT',
                                                               available_modes))

        # android arguments
        available_build_types = [ 'ndk-build', 'none'] # TODO, support cmake
        self._build_type = self.check_param(args.build_type, 'ndk-build', available_build_types,
                                          MultiLanguage.get_string('COMPILE_ERROR_WRONG_BUILD_TYPE_FMT',
                                                                   available_build_types))
        self._no_apk = args.no_apk
        self._no_sign = args.no_sign

        self.app_abi = None
        if args.app_abi:
            self.app_abi = " ".join(args.app_abi.split(":"))

        self.cppflags = None
        if args.cppflags:
            self.cppflags = args.cppflags

        self.ndk_toolchain = None
        if args.toolchain:
            self.ndk_toolchain = args.toolchain

        # Win32 arguments
        self.vs_version = args.vs_version

        # iOS/Mac arguments
        self.xcode_target_name = None
        if args.target_name is not None:
            self.xcode_target_name = args.target_name

        if args.compile_script is not None:
            self._compile_script = bool(args.compile_script)
        else:
            self._compile_script = (self._mode == "release")

        self._ap = args.android_platform

        if args.jobs is not None:
            self._jobs = args.jobs
        else:
            self._jobs = self.get_num_of_cpu()
        self._has_sourcemap = args.source_map
        self._web_advanced = args.advanced
        self._no_res = args.no_res

        if args.output_dir is None:
            self._output_dir = self._get_output_dir()
        else:
            if os.path.isabs(args.output_dir):
                self._output_dir = args.output_dir
            else:
                self._output_dir = os.path.abspath(args.output_dir)

        self._sign_id = args.sign_id

        if self._project._is_lua_project():
            self._lua_encrypt = args.lua_encrypt
            self._lua_encrypt_key = args.lua_encrypt_key
            self._lua_encrypt_sign = args.lua_encrypt_sign

        self.end_warning = ""
        self._gen_custom_step_args()

    def check_param(self, value, default_value, available_values, error_msg, ignore_case=True):
        if value is None:
            return default_value

        if ignore_case:
            check_value = value.lower()
            right_values = []
            for v in available_values:
                right_values.append(v.lower())
        else:
            check_value = value
            right_values = available_values

        if check_value in right_values:
            return check_value
        else:
            raise cocos.CCPluginError(error_msg, cocos.CCPluginError.ERROR_WRONG_ARGS)

    def get_num_of_cpu(self):
        try:
            return multiprocessing.cpu_count()
        except Exception:
            print MultiLanguage.get_string('COMPILE_DETECT_CPU_FAILED')
            return 1

    def _get_output_dir(self):
        project_dir = self._project.get_project_dir()
        cur_platform = self._platforms.get_current_platform()
        if self._project._is_script_project():
            if self._project._is_js_project() and self._platforms.is_web_active():
                cur_platform = CCPluginCompile.WEB_PLATFORM_FOLDER_NAME

            if self._mode == 'debug':
                output_dir = os.path.join(project_dir, CCPluginCompile.OUTPUT_DIR_SCRIPT_DEBUG, cur_platform)
            else:
                output_dir = os.path.join(project_dir, CCPluginCompile.OUTPUT_DIR_SCRIPT_RELEASE, cur_platform)
        else:
            output_dir = os.path.join(project_dir, CCPluginCompile.OUTPUT_DIR_NATIVE, self._mode, cur_platform)

        return output_dir

    def _gen_custom_step_args(self):
        self._custom_step_args = {
            "project-path": self._project.get_project_dir(),
            "platform-project-path": self._platforms.project_path(),
            "build-mode": self._mode,
            "output-dir": self._output_dir
        }

        if self._platforms.is_android_active():
            self._custom_step_args["ndk-build-type"] = self._build_type

    def _build_cfg_path(self):
        cur_cfg = self._platforms.get_current_config()
        if self._platforms.is_win32_active():
            if cur_cfg.build_cfg_path is not None:
                project_dir = self._project.get_project_dir()
                ret = os.path.join(project_dir, cur_cfg.build_cfg_path)
            else:
                ret = self._platforms.project_path()
        elif self._platforms.is_ios_active():
            ret = os.path.join(self._platforms.project_path(), "ios")
        elif self._platforms.is_mac_active():
            ret = os.path.join(self._platforms.project_path(), "mac")
        else:
            ret = self._platforms.project_path()

        return ret

    def _update_build_cfg(self):
        build_cfg_dir = self._build_cfg_path()
        cfg_file_path = os.path.join(build_cfg_dir, CCPluginCompile.BUILD_CONFIG_FILE)
        if not os.path.isfile(cfg_file_path):
            return

        key_of_copy = None
        key_of_must_copy = None
        if self._platforms.is_android_active():
            from build_android import AndroidBuilder
            key_of_copy = AndroidBuilder.CFG_KEY_COPY_TO_ASSETS
            key_of_must_copy = AndroidBuilder.CFG_KEY_MUST_COPY_TO_ASSERTS
        elif self._platforms.is_win32_active():
            key_of_copy = CCPluginCompile.CFG_KEY_WIN32_COPY_FILES
            key_of_must_copy = CCPluginCompile.CFG_KEY_WIN32_MUST_COPY_FILES

        if key_of_copy is None and key_of_must_copy is None:
            return

        try:
            outfile = None
            open_file = open(cfg_file_path)
            cfg_info = json.load(open_file)
            open_file.close()
            open_file = None
            changed = False
            if key_of_copy is not None:
                if cfg_info.has_key(key_of_copy):
                    src_list = cfg_info[key_of_copy]
                    ret_list = self._convert_cfg_list(src_list, build_cfg_dir)
                    cfg_info[CCPluginCompile.CFG_KEY_COPY_RESOURCES] = ret_list
                    del cfg_info[key_of_copy]
                    changed = True

            if key_of_must_copy is not None:
                if cfg_info.has_key(key_of_must_copy):
                    src_list = cfg_info[key_of_must_copy]
                    ret_list = self._convert_cfg_list(src_list, build_cfg_dir)
                    cfg_info[CCPluginCompile.CFG_KEY_MUST_COPY_RESOURCES] = ret_list
                    del cfg_info[key_of_must_copy]
                    changed = True

            if changed:
                # backup the old-cfg
                split_list = os.path.splitext(CCPluginCompile.BUILD_CONFIG_FILE)
                file_name = split_list[0]
                ext_name = split_list[1]
                bak_name = file_name + "-for-v0.1" + ext_name
                bak_file_path = os.path.join(build_cfg_dir, bak_name)
                if os.path.exists(bak_file_path):
                    os.remove(bak_file_path)
                os.rename(cfg_file_path, bak_file_path)

                # write the new data to file
                with open(cfg_file_path, 'w') as outfile:
                    json.dump(cfg_info, outfile, sort_keys = True, indent = 4)
                    outfile.close()
                    outfile = None
        finally:
            if open_file is not None:
                open_file.close()

            if outfile is not None:
                outfile.close()

    def _convert_cfg_list(self, src_list, build_cfg_dir):
        ret = []
        for element in src_list:
            ret_element = {}
            if str(element).endswith("/"):
                sub_str = element[0:len(element)-1]
                ret_element["from"] = sub_str
                ret_element["to"] = ""
            else:
                element_full_path = os.path.join(build_cfg_dir, element)
                if os.path.isfile(element_full_path):
                    to_dir = ""
                else:
                    to_dir = os.path.basename(element)
                ret_element["from"] = element
                ret_element["to"] = to_dir

            ret.append(ret_element)

        return ret

    def _is_debug_mode(self):
        return self._mode == 'debug'

    def _remove_file_with_ext(self, work_dir, ext):
        file_list = os.listdir(work_dir)
        for f in file_list:
            full_path = os.path.join(work_dir, f)
            if os.path.isdir(full_path):
                self._remove_file_with_ext(full_path, ext)
            elif os.path.isfile(full_path):
                name, cur_ext = os.path.splitext(f)
                if cur_ext == ext:
                    os.remove(full_path)

    def compile_lua_scripts(self, src_dir, dst_dir, build_64):
        if not self._project._is_lua_project():
            return False

        if not self._compile_script and not self._lua_encrypt:
            return False

        cocos_cmd_path = os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), "cocos")
        rm_ext = ".lua"
        compile_cmd = "\"%s\" luacompile -s \"%s\" -d \"%s\"" % (cocos_cmd_path, src_dir, dst_dir)

        if not self._compile_script:
            compile_cmd = "%s --disable-compile" % compile_cmd
        elif build_64:
            compile_cmd = "%s --bytecode-64bit" % compile_cmd

        if self._lua_encrypt:
            add_para = ""
            if self._lua_encrypt_key is not None:
                add_para = "%s -k %s" % (add_para, self._lua_encrypt_key)

            if self._lua_encrypt_sign is not None:
                add_para = "%s -b %s" % (add_para, self._lua_encrypt_sign)

            compile_cmd = "%s -e %s" % (compile_cmd, add_para)

        # run compile command
        self._run_cmd(compile_cmd)

        # remove the source scripts
        self._remove_file_with_ext(dst_dir, rm_ext)

        return True

    def compile_js_scripts(self, src_dir, dst_dir):
        if not self._project._is_js_project():
            return False

        if not self._compile_script:
            return False

        cocos_cmd_path = os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), "cocos")
        rm_ext = ".js"
        compile_cmd = "\"%s\" jscompile -s \"%s\" -d \"%s\"" % (cocos_cmd_path, src_dir, dst_dir)

        # run compile command
        self._run_cmd(compile_cmd)

        # remove the source scripts
        self._remove_file_with_ext(dst_dir, rm_ext)
        return True

    def add_warning_at_end(self, warning_str):
        if warning_str is None or len(warning_str) == 0:
            return
        self.end_warning = "%s\n%s" % (self.end_warning, warning_str)

    def is_valid_path(self, p):
        if (p is not None) and os.path.exists(p):
            ret = True
        else:
            ret = False

        return ret

    def get_engine_version_num(self):
        # 1. get engine version from .cocos_project.json
        engine_ver_str = self._project.get_proj_config(cocos_project.Project.KEY_ENGINE_VERSION)

        # 2. engine version is not found. find from source file
        if engine_ver_str is None:
            engine_dir = self.get_engine_dir()
            if engine_dir is not None:
                engine_ver_str = utils.get_engine_version(engine_dir)

        if engine_ver_str is None:
            return None

        version_pattern = r'cocos2d-x-([\d]+)\.([\d]+)'
        match = re.match(version_pattern, engine_ver_str)
        if match:
            return ((int)(match.group(1)), (int)(match.group(2)))
        else:
            return None

    def build_android(self):
        if not self._platforms.is_android_active():
            return

        project_dir = self._project.get_project_dir()
        build_mode = self._mode
        output_dir = self._output_dir

        # get the android project path
        cfg_obj = self._platforms.get_current_config()
        project_android_dir = cfg_obj.proj_path


        ide_name = 'Android Studio'
        cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_ANDROID_PROJPATH_FMT', (ide_name, project_android_dir)))

        # Check whether the gradle of the project is support ndk or not
        # Get the engine version of the project
        engine_version_num = self.get_engine_version_num()
        if engine_version_num is None:
            raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_UNKNOWN_ENGINE_VERSION'))

        # Gradle supports NDK build from engine 3.15
        main_ver = engine_version_num[0]
        minor_ver = engine_version_num[1]
        if main_ver > 3 or (main_ver == 3 and minor_ver >= 15):
            gradle_support_ndk = True
            

        from build_android import AndroidBuilder
        builder = AndroidBuilder(self._verbose, project_android_dir,
                                 self._no_res, self._project, self._mode, self._build_type,
                                 self.app_abi, gradle_support_ndk)

        args_ndk_copy = self._custom_step_args.copy()
        target_platform = self._platforms.get_current_platform()

        # update the project with the android platform
        builder.update_project(self._ap)

        if not self._project._is_script_project() or self._project._is_native_support():
            if self._build_type != "none" and not gradle_support_ndk:
                # build native code
                cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILD_NATIVE'))
                ndk_build_param = [
                    "-j%s" % self._jobs
                ]

                if self.app_abi:
                    abi_param = "APP_ABI=\"%s\"" % self.app_abi
                    ndk_build_param.append(abi_param)

                if self.ndk_toolchain:
                    toolchain_param = "NDK_TOOLCHAIN=%s" % self.ndk_toolchain
                    ndk_build_param.append(toolchain_param)

                self._project.invoke_custom_step_script(cocos_project.Project.CUSTOM_STEP_PRE_NDK_BUILD, target_platform, args_ndk_copy)

                modify_mk = False
                app_mk = os.path.join(project_android_dir, "app/jni/Application.mk")

                mk_content = None
                if self.cppflags and os.path.exists(app_mk):
                    # record the content of Application.mk
                    f = open(app_mk)
                    mk_content = f.read()
                    f.close()

                    # Add cpp flags
                    f = open(app_mk, "a")
                    f.write("\nAPP_CPPFLAGS += %s" % self.cppflags)
                    f.close()
                    modify_mk = True

                try:
                    builder.do_ndk_build(ndk_build_param, self._mode, self._build_type, self)
                except Exception as e:
                    if e.__class__.__name__ == 'CCPluginError':
                        raise e
                    else:
                        raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_NDK_BUILD_FAILED'),
                                                  cocos.CCPluginError.ERROR_BUILD_FAILED)
                finally:
                    # roll-back the Application.mk
                    if modify_mk:
                        f = open(app_mk, "w")
                        f.write(mk_content)
                        f.close()

                self._project.invoke_custom_step_script(cocos_project.Project.CUSTOM_STEP_POST_NDK_BUILD, target_platform, args_ndk_copy)

        # build apk
        if not self._no_apk:
            cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILD_APK'))
        self.apk_path = builder.do_build_apk(build_mode, self._no_apk, self._no_sign, output_dir, self._custom_step_args, self._ap, self)
        self.android_package, self.android_activity = builder.get_apk_info()

        cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILD_SUCCEED'))

    def check_ios_mac_build_depends(self):
        version = cocos.get_xcode_version()

        if version <= '5':
            message = MultiLanguage.get_string('COMPILE_ERROR_UPDATE_XCODE')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_TOOLS_NOT_FOUND)

        cfg_obj = self._platforms.get_current_config()
        if cfg_obj.proj_file is not None:
            xcodeproj_name = cfg_obj.proj_file
            name = os.path.basename(xcodeproj_name)
        else:
            name, xcodeproj_name = self.checkFileByExtention(".xcodeproj", self._platforms.project_path())
        if not xcodeproj_name:
            message = MultiLanguage.get_string('COMPILE_ERROR_XCODEPROJ_NOT_FOUND')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PARSE_FILE)

        self.project_name = name
        self.xcodeproj_name = xcodeproj_name
        project_dir = self._platforms.project_path()
        podfile = os.path.join(project_dir, 'Podfile')
        self.xcworkspace = os.path.join(project_dir, name+'.xcworkspace')
        self.cocoapods = os.path.exists(self.xcworkspace) and os.path.exists(podfile)
        if self.cocoapods:
          cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_USE_COCOAPODS'))


    def append_xcpretty_if_installed(self, command):
        if not cocos.os_is_mac():
            return command;
        from distutils import spawn
        if spawn.find_executable("xcpretty") == None:
            return command
        return command + " | xcpretty"


    def _remove_res(self, target_path):
        build_cfg_dir = self._build_cfg_path()
        cfg_file = os.path.join(build_cfg_dir, CCPluginCompile.BUILD_CONFIG_FILE)
        if os.path.exists(cfg_file) and os.path.isfile(cfg_file):
            # have config file
            open_file = open(cfg_file)
            cfg_info = json.load(open_file)
            open_file.close()
            if cfg_info.has_key("remove_res"):
                remove_list = cfg_info["remove_res"]
                for f in remove_list:
                    res = os.path.join(target_path, f)
                    if os.path.isdir(res):
                        # is a directory
                        if f.endswith('/'):
                            # remove files & dirs in it
                            for sub_file in os.listdir(res):
                                sub_file_fullpath = os.path.join(res, sub_file)
                                if os.path.isfile(sub_file_fullpath):
                                    os.remove(sub_file_fullpath)
                                elif os.path.isdir(sub_file_fullpath):
                                    shutil.rmtree(sub_file_fullpath)
                        else:
                            # remove the dir
                            shutil.rmtree(res)
                    elif os.path.isfile(res):
                        # is a file, remove it
                        os.remove(res)

    def get_engine_dir(self):
        engine_dir = self._project.get_proj_config(CCPluginCompile.PROJ_CFG_KEY_ENGINE_DIR)
        if engine_dir is None:
            proj_dir = self._project.get_project_dir()
            if self._project._is_js_project():
                check_dir = os.path.join(proj_dir, "frameworks", "cocos2d-x")
                if os.path.isdir(check_dir):
                    # the case for jsb in cocos2d-x engine
                    engine_dir = check_dir
                else:
                    # the case for jsb in cocos2d-js engine
                    engine_dir = proj_dir
            elif self._project._is_lua_project():
                engine_dir = os.path.join(proj_dir, "frameworks", "cocos2d-x")
            else:
                engine_dir = os.path.join(proj_dir, "cocos2d")
        else:
            engine_dir = os.path.join(self._project.get_project_dir(), engine_dir)

        return engine_dir

    def backup_dir(self, dir_path):
        backup_dir = "%s%s" % (dir_path, CCPluginCompile.BACKUP_SUFFIX)
        if os.path.exists(backup_dir):
            shutil.rmtree(backup_dir)
        shutil.copytree(dir_path, backup_dir)

    def reset_backup_dir(self, dir_path):
        backup_dir = "%s%s" % (dir_path, CCPluginCompile.BACKUP_SUFFIX)
        if os.path.exists(dir_path):
            shutil.rmtree(dir_path)
        os.rename(backup_dir, dir_path)

    def get_engine_js_dir(self):
        engine_js_dir = None
        isFound = False

        check_script_dir = os.path.join(self._project.get_project_dir(), "script")
        if os.path.isdir(check_script_dir):
            # JS script already copied into the project dir
            engine_js_dir = check_script_dir
            isFound = True
        else:
            for js_dir in CCPluginCompile.ENGINE_JS_DIRS:
                engine_js_dir = os.path.join(self.get_engine_dir(), js_dir)
                if os.path.isdir(engine_js_dir):
                    isFound = True
                    break

        if isFound:
            return engine_js_dir
        else:
            return None

    def build_ios(self):
        if not self._platforms.is_ios_active():
            return

        if not cocos.os_is_mac():
            raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_BUILD_ON_MAC'),
                                      cocos.CCPluginError.ERROR_WRONG_ARGS)
              
        if self._sign_id is not None:
            cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_IOS_SIGN_FMT', self._sign_id))
            self.use_sdk = 'iphoneos'
        else:
            self.use_sdk = 'iphonesimulator'

        self.check_ios_mac_build_depends()

        ios_project_dir = self._platforms.project_path()
        output_dir = self._output_dir

        projectPath = os.path.join(ios_project_dir, self.xcodeproj_name)
        pbxprojectPath = os.path.join(projectPath, "project.pbxproj")

        f = file(pbxprojectPath)
        contents = f.read()

        section = re.search(r"Begin PBXProject section.*End PBXProject section", contents, re.S)

        if section is None:
            message = MultiLanguage.get_string('COMPILE_ERROR_NO_IOS_TARGET')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PARSE_FILE)

        targets = re.search(r"targets = (.*);", section.group(), re.S)
        if targets is None:
            message = MultiLanguage.get_string('COMPILE_ERROR_NO_IOS_TARGET')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PARSE_FILE)

        targetName = None
        if self.xcode_target_name is not None:
            targetName = self.xcode_target_name
        else:
            cfg_obj = self._platforms.get_current_config()
            if cfg_obj.target_name is not None:
                targetName = cfg_obj.target_name
            else:
                names = re.split("\*", targets.group())
                for name in names:
                    if "iOS" in name or "-mobile" in name:
                        targetName = str.strip(name)
                        break

        if targetName is None:
            message = MultiLanguage.get_string('COMPILE_ERROR_NO_IOS_TARGET')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PARSE_FILE)

        if os.path.isdir(output_dir):
            target_app_dir = os.path.join(output_dir, "%s.app" % targetName)
            if os.path.isdir(target_app_dir):
                if os.path.islink(target_app_dir):
                    os.unlink(target_app_dir)
                else:
                    shutil.rmtree(target_app_dir)
            elif os.path.isfile(target_app_dir):
                if os.path.islink(target_app_dir):
                    os.unlink(target_app_dir)
                else:
                    os.remove(target_app_dir)

        # is script project, check whether compile scripts or not
        need_reset_dir = False
        if self._project._is_script_project():
            script_src_dir = os.path.join(self._project.get_project_dir(), "src")

            if self._project._is_js_project() and self._compile_script:
                # backup the source scripts
                self.backup_dir(script_src_dir)
                self.compile_js_scripts(script_src_dir, script_src_dir)

                # js project need compile the js files in engine
                engine_js_dir = self.get_engine_js_dir()
                if engine_js_dir is not None:
                    self.backup_dir(engine_js_dir)
                    self.compile_js_scripts(engine_js_dir, engine_js_dir)
                need_reset_dir = True

            if self._project._is_lua_project():
                self.backup_dir(script_src_dir)
                # create 64-bit folder and build 64-bit bytecode
                # should build 64-bit first because `script_src_dir` will be deleted when building 32-bit bytecode 
                folder_64bit = os.path.join(script_src_dir, '64bit')
                self.compile_lua_scripts(script_src_dir, folder_64bit, True)
                # build 32-bit bytecode
                self.compile_lua_scripts(script_src_dir, script_src_dir, False)
                need_reset_dir = True
        try:
            cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILDING'))

            xcode_version = cocos.get_xcode_version()
            xcode9_and_upper = cocos.version_compare(xcode_version,">=",9)
            # Xcode 9+ need to use `-scheme`
            use_scheme = self.cocoapods or ( xcode9_and_upper and  self._sign_id)

            command = ' '.join([
                "xcodebuild",
                "-workspace" if self.cocoapods else "-project",
                "\"%s\"" % self.xcworkspace if self.cocoapods else projectPath,
                "-configuration",
                "%s" % 'Debug' if self._mode == 'debug' else 'Release',
                "-scheme" if use_scheme else "-target",
                "\"%s\"" % targetName,
                "%s" % "-arch \"x86_64\"" if self.use_sdk == 'iphonesimulator' else '',
                "-sdk",
                "%s" % self.use_sdk,
                "CONFIGURATION_BUILD_DIR=\"%s\"" % (output_dir),
                "%s" % "VALID_ARCHS=\"i386 x86_64\"" if self.use_sdk == 'iphonesimulator' else ''
                ])

            # PackageApplication is removed since xcode 8.3, should use new method to generate .ipa
            # should generate .xcarchive first, then generate .ipa
            use_new_ipa_method = cocos.version_compare(xcode_version,">=",8.3)

            if self._sign_id is not None:
                if use_new_ipa_method:
                    archive_path = os.path.join(output_dir, "%s.xcarchive" % targetName)
                    command = "%s CODE_SIGN_IDENTITY=\"%s\" -archivePath '%s' archive" % (command, self._sign_id, archive_path)
                else:
                    command = "%s CODE_SIGN_IDENTITY=\"%s\"" % (command, self._sign_id)

            command = self.append_xcpretty_if_installed(command)
            self._run_cmd(command)

            filelist = os.listdir(output_dir)
            for filename in filelist:
                name, extention = os.path.splitext(filename)
                if extention == '.a':
                    filename = os.path.join(output_dir, filename)
                    os.remove(filename)

            self._iosapp_path = os.path.join(output_dir, "%s.app" % targetName)
            if self._no_res:
                self._remove_res(self._iosapp_path)

            if self._sign_id is not None:
                # generate the ipa
                ipa_path = os.path.join(output_dir, "%s.ipa" % targetName)
                if use_new_ipa_method:
                    # find the path of `exportoptions.plist`
                    export_options_path = self._get_export_options_plist_path()
                    if export_options_path is None:
                        cocos.Logging.error('Can not find exportoptions.plist.')
                        raise Exception('Can not find exportoptions.plist.')


                    archive_path = os.path.join(output_dir, "%s.xcarchive" % targetName)
                    ipa_cmd = "xcodebuild -exportArchive -archivePath '%s' -exportPath '%s' -exportOptionsPlist '%s'" % (archive_path, output_dir, export_options_path)
                    self._run_cmd(ipa_cmd)
                else:
                    ipa_cmd = "xcrun -sdk %s PackageApplication -v \"%s\" -o \"%s\"" % (self.use_sdk, self._iosapp_path, ipa_path)
                    self._run_cmd(ipa_cmd)

            cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILD_SUCCEED'))
        except Exception, e:
            print str(e)
            raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_BUILD_FAILED'),
                                      cocos.CCPluginError.ERROR_BUILD_FAILED)
        finally:
            # is script project & need reset dirs
            if need_reset_dir:
                script_src_dir = os.path.join(self._project.get_project_dir(), "src")
                self.reset_backup_dir(script_src_dir)

                if self._project._is_js_project():
                    engine_js_dir = self.get_engine_js_dir()
                    if engine_js_dir is not None:
                        self.reset_backup_dir(engine_js_dir)

    def _get_export_options_plist_path(self):
        project_dir = self._project.get_project_dir()

        possible_sub_paths = [ 'proj.ios', 'proj.ios_mac/ios', 'frameworks/runtime-src/proj.ios_mac/ios' ]
        ios_project_dir = None
        for sub_path in possible_sub_paths:
            ios_project_dir = os.path.join(project_dir, sub_path)
            if os.path.exists(ios_project_dir):
                break

        if ios_project_dir is None:
            return None

        return os.path.join(ios_project_dir, 'exportOptions.plist')

    def build_mac(self):
        if not self._platforms.is_mac_active():
            return

        if not cocos.os_is_mac():
            raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_BUILD_ON_MAC'),
                                      cocos.CCPluginError.ERROR_WRONG_ARGS)

        self.check_ios_mac_build_depends()

        mac_project_dir = self._platforms.project_path()
        output_dir = self._output_dir

        projectPath = os.path.join(mac_project_dir, self.xcodeproj_name)
        pbxprojectPath = os.path.join(projectPath, "project.pbxproj")

        f = file(pbxprojectPath)
        contents = f.read()

        section = re.search(
            r"Begin PBXProject section.*End PBXProject section",
            contents,
            re.S
        )

        if section is None:
            message = MultiLanguage.get_string('COMPILE_ERROR_NO_MAC_TARGET')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PARSE_FILE)

        targets = re.search(r"targets = (.*);", section.group(), re.S)
        if targets is None:
            message = MultiLanguage.get_string('COMPILE_ERROR_NO_MAC_TARGET')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PARSE_FILE)

        targetName = None
        if self.xcode_target_name is not None:
            targetName = self.xcode_target_name
        else:
            cfg_obj = self._platforms.get_current_config()
            if cfg_obj.target_name is not None:
                targetName = cfg_obj.target_name
            else:
                names = re.split("\*", targets.group())
                for name in names:
                    if "Mac" in name or "-desktop" in name:
                        targetName = str.strip(name)
                        break

        if targetName is None:
            message = MultiLanguage.get_string('COMPILE_ERROR_NO_MAC_TARGET')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PARSE_FILE)

        if os.path.isdir(output_dir):
            target_app_dir = os.path.join(output_dir, "%s.app" % targetName)
            if os.path.isdir(target_app_dir):
                shutil.rmtree(target_app_dir)

        # is script project, check whether compile scripts or not
        need_reset_dir = False
        if self._project._is_script_project():
            script_src_dir = os.path.join(self._project.get_project_dir(), "src")

            if self._project._is_js_project() and self._compile_script:
                # backup the source scripts
                self.backup_dir(script_src_dir)
                self.compile_js_scripts(script_src_dir, script_src_dir)

                # js project need compile the js files in engine
                engine_js_dir = self.get_engine_js_dir()
                if engine_js_dir is not None:
                    self.backup_dir(engine_js_dir)
                    self.compile_js_scripts(engine_js_dir, engine_js_dir)
                need_reset_dir = True

            if self._project._is_lua_project():
                self.backup_dir(script_src_dir)
                # mac only support 64-bit bytecode
                folder_64bit = os.path.join(script_src_dir, '64bit')
                self.compile_lua_scripts(script_src_dir, folder_64bit, True)
                need_reset_dir = True

        try:
            cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILDING'))

            command = ' '.join([
                "xcodebuild",
                "-workspace" if self.cocoapods else "-project",
                "\"%s\"" % self.xcworkspace if self.cocoapods else projectPath,
                "-configuration",
                "%s" % 'Debug' if self._mode == 'debug' else 'Release',
                "-scheme" if self.cocoapods else "-target",
                "\"%s\"" % targetName,
                "CONFIGURATION_BUILD_DIR=\"%s\"" % (output_dir)
                ])

            command = self.append_xcpretty_if_installed(command)
            self._run_cmd(command)

            self.target_name = targetName
            filelist = os.listdir(output_dir)
            for filename in filelist:
                name, extention = os.path.splitext(filename)
                if extention == '.a':
                    filename = os.path.join(output_dir, filename)
                    os.remove(filename)

            self._macapp_path = os.path.join(output_dir, "%s.app" % targetName)
            if self._no_res:
                resource_path = os.path.join(self._macapp_path, "Contents", "Resources")
                self._remove_res(resource_path)

            cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILD_SUCCEED'))
        except:
            raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_BUILD_FAILED'),
                                      cocos.CCPluginError.ERROR_BUILD_FAILED)
        finally:
            # is script project & need reset dirs
            if need_reset_dir:
                script_src_dir = os.path.join(self._project.get_project_dir(), "src")
                self.reset_backup_dir(script_src_dir)

                if self._project._is_js_project():
                    engine_js_dir = self.get_engine_js_dir()
                    if engine_js_dir is not None:
                        self.reset_backup_dir(engine_js_dir)

    # Get the required VS versions from the engine version of project
    def get_required_vs_versions(self):
        # get the engine version string
        ret = []

        # 1. get engine version from .cocos_project.json
        engine_ver_str = self._project.get_proj_config(cocos_project.Project.KEY_ENGINE_VERSION)

        # 2. engine version is not found. find from source file
        if engine_ver_str is None:
            engine_dir = self.get_engine_dir()
            if engine_dir is not None:
                engine_ver_str = utils.get_engine_version(engine_dir)

        if engine_ver_str is None:
            return ret

        # get the float value of engine version
        version_pattern = r'cocos2d-x[^0-9]*([\d]+)\.([\d]+)'
        match = re.match(version_pattern, engine_ver_str)
        if match:
            major_ver = int(match.group(1))
            minor_ver = int(match.group(2))
        else:
            major_ver = -1
            minor_ver = -1

        if major_ver < 0:
            return ret

        if (major_ver > 3) or (major_ver == 3 and minor_ver >= 7):
            ret = [ 2013, 2015, 2017 ]
        else:
            ret = [ 2012, 2013 ]

        return ret

    def get_min_vs_version(self):
        if self._platforms.is_metro_active():
            # metro project required VS 2013
            return 2013
        else:
            # win32 project required VS 2012
            return 2012

    def get_available_devenv(self, required_versions, min_ver, specify_vs_ver=None):
        if required_versions is None or len(required_versions) == 0:
            if specify_vs_ver is None:
                # Not specify VS version, find newest version
                needUpgrade, commandPath = utils.get_newest_devenv(min_ver)
            else:
                # Have specified VS version
                if specify_vs_ver < min_ver:
                    # Specified version is lower than required, raise error
                    raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_LOW_VS_VER'),
                                              cocos.CCPluginError.ERROR_WRONG_ARGS)
                else:
                    # Get the specified VS
                    commandPath = utils.get_devenv_path(specify_vs_ver)
                    if specify_vs_ver > min_ver:
                        needUpgrade = True
                    else:
                        needUpgrade = False
        else:
            needUpgrade = False
            if specify_vs_ver is None:
                # find VS in required versions
                commandPath = None
                for v in required_versions:
                    commandPath = utils.get_devenv_path(v)
                    if commandPath is not None:
                        break
            else:
                # use specified VS version
                if specify_vs_ver in required_versions:
                    commandPath = utils.get_devenv_path(specify_vs_ver)
                else:
                    raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_WRONG_VS_VER_FMT', specify_vs_ver),
                                              cocos.CCPluginError.ERROR_WRONG_ARGS)

        if commandPath is None:
            message = MultiLanguage.get_string('COMPILE_ERROR_VS_NOT_FOUND')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_TOOLS_NOT_FOUND)

        return (needUpgrade, commandPath)

    def get_available_msbuild(self, required_versions, min_ver, specify_vs_ver=None):
        if required_versions is None or len(required_versions) == 0:
            if specify_vs_ver is None:
                # Not specify VS version, find newest version
                commandPath = utils.get_newest_msbuild(min_ver)
            else:
                # Have specified VS version
                if specify_vs_ver < min_ver:
                    # Specified version is lower than required, raise error
                    raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_LOW_VS_VER'),
                                              cocos.CCPluginError.ERROR_WRONG_ARGS)
                else:
                    # Get the specified VS
                    commandPath = utils.get_msbuild_path(specify_vs_ver)
        else:
            if specify_vs_ver is None:
                # find VS in required versions
                commandPath = None
                for v in required_versions:
                    commandPath = utils.get_msbuild_path(v)
                    if commandPath is not None:
                        break
            else:
                # use specified VS version
                if specify_vs_ver in required_versions:
                    commandPath = utils.get_msbuild_path(specify_vs_ver)
                else:
                    raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_WRONG_VS_VER_FMT', specify_vs_ver),
                                              cocos.CCPluginError.ERROR_WRONG_ARGS)

        if commandPath is None:
            message = MultiLanguage.get_string('COMPILE_ERROR_VS_NOT_FOUND')
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_TOOLS_NOT_FOUND)

        return commandPath

    def build_vs_project(self, sln_file, project_name, build_mode, specify_vs_ver=None):
        required_versions = self.get_required_vs_versions()
        min_ver = self.get_min_vs_version()
        if required_versions is None or len(required_versions) == 0:
            msg_version = min_ver
        else:
            msg_version = required_versions
        cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_REQUIRED_VS_FMT', msg_version))

        needUpgrade, commandPath = self.get_available_devenv(required_versions, min_ver, specify_vs_ver)
        if os.path.exists(commandPath):
            # upgrade projects
            if needUpgrade:
                commandUpgrade = ' '.join([
                    "\"%s\"" % commandPath,
                    "\"%s\"" % sln_file,
                    "/Upgrade"
                ])
                self._run_cmd(commandUpgrade)

            # build the project
            commands = ' '.join([
                "\"%s\"" % commandPath,
                "\"%s\"" % sln_file,
                "/Build \"%s\"" % build_mode,
                "/Project \"%s\"" % project_name
            ])

            self._run_cmd(commands)
        else:
            cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_DEVENV_NOT_FOUND'))

            msbuild_path = self.get_available_msbuild(required_versions, min_ver, specify_vs_ver)

            if msbuild_path:
                msbuild_path = os.path.join(msbuild_path, 'MSBuild.exe')
                cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_FIND_MSBUILD_FMT', msbuild_path))

                job_number = 2
                build_command = ' '.join([
                    '\"%s\"' % msbuild_path,
                    '\"%s\"' % sln_file,
                    '/target:%s' % project_name,
                    '/property:Configuration=%s' % build_mode,
                    '/maxcpucount:%s' % job_number
                    ])

                self._run_cmd(build_command)
            else:
                raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_MSDBUILD_NOT_FOUND'),
                                          cocos.CCPluginError.ERROR_TOOLS_NOT_FOUND)

        cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILD_SUCCEED'))

    def build_win32(self):
        if not self._platforms.is_win32_active():
            return

        if not cocos.os_is_win32():
            raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_BUILD_ON_WIN'),
                                      cocos.CCPluginError.ERROR_WRONG_ARGS)

        win32_projectdir = self._platforms.project_path()
        output_dir = self._output_dir

        cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILDING'))

        # get the solution file & project name
        cfg_obj = self._platforms.get_current_config()
        if cfg_obj.sln_file is not None:
            sln_name = cfg_obj.sln_file
            if cfg_obj.project_name is None:
                raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_CFG_NOT_FOUND_FMT',
                                                                   (cocos_project.Win32Config.KEY_PROJECT_NAME,
                                                                    cocos_project.Win32Config.KEY_SLN_FILE,
                                                                    cocos_project.Project.CONFIG)),
                                          cocos.CCPluginError.ERROR_WRONG_CONFIG)
            else:
                name = cfg_obj.project_name
        else:
            name, sln_name = self.checkFileByExtention(".sln", win32_projectdir)
            if not sln_name:
                message = MultiLanguage.get_string('COMPILE_ERROR_SLN_NOT_FOUND')
                raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PATH_NOT_FOUND)

        # build the project
        self.project_name = name
        projectPath = os.path.join(win32_projectdir, sln_name)
        build_mode = 'Debug' if self._is_debug_mode() else 'Release'
        self.build_vs_project(projectPath, self.project_name, build_mode, self.vs_version)

        # copy files
        build_folder_name = "%s.win32" % build_mode
        build_folder_path = os.path.join(win32_projectdir, build_folder_name)
        if not os.path.isdir(build_folder_path):
            message = MultiLanguage.get_string('COMPILE_ERROR_BUILD_PATH_NOT_FOUND_FMT', build_folder_path)
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PATH_NOT_FOUND)

        # remove the files in output dir (keep the exe files)
        if os.path.exists(output_dir):
            output_files = os.listdir(output_dir)
            for element in output_files:
                ele_full_path = os.path.join(output_dir, element)
                if os.path.isfile(ele_full_path):
                    base_name, file_ext = os.path.splitext(element)
                    if not file_ext == ".exe":
                        os.remove(ele_full_path)
                elif os.path.isdir(ele_full_path):
                    shutil.rmtree(ele_full_path)

        # create output dir if it not existed
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        if cfg_obj.exe_out_dir is None:
            exe_out_dir = build_folder_path
        else:
            exe_out_dir = os.path.join(build_folder_path, cfg_obj.exe_out_dir)

        # copy exe
        files = os.listdir(exe_out_dir)
        proj_exe_name = "%s.exe" % self.project_name
        for filename in files:
            if filename == proj_exe_name:
                file_path = os.path.join(exe_out_dir, filename)
                cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_COPYING_FMT', filename))
                shutil.copy(file_path, output_dir)
                break

        # copy dll
        files = os.listdir(build_folder_path)
        for filename in files:
            name, ext = os.path.splitext(filename)
            if ext == '.dll':
                file_path = os.path.join(build_folder_path, filename)
                cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_COPYING_FMT', filename))
                shutil.copy(file_path, output_dir)

        # copy lua files & res
        self._copy_resources(output_dir)

        # check the project config & compile the script files
        if self._project._is_js_project():
            self.compile_js_scripts(output_dir, output_dir)

        if self._project._is_lua_project():
            # windows only support 32-bit bytecode
            self.compile_lua_scripts(output_dir, output_dir, False)

        self.run_root = output_dir

    def build_web(self):
        if not self._platforms.is_web_active():
            return

        project_dir = self._platforms.project_path()

        # store env for run
        cfg_obj = self._platforms.get_current_config()
        if cfg_obj.run_root_dir is not None:
            self.run_root = cfg_obj.run_root_dir
        else:
            self.run_root = project_dir

        if cfg_obj.sub_url is not None:
            self.sub_url = cfg_obj.sub_url
        else:
            self.sub_url = '/'

        output_dir = CCPluginCompile.OUTPUT_DIR_SCRIPT_RELEASE
        if self._is_debug_mode():
            output_dir = CCPluginCompile.OUTPUT_DIR_SCRIPT_DEBUG
            if not self._web_advanced:
                return

        self.sub_url = '%s%s/%s/' % (self.sub_url, output_dir, CCPluginCompile.WEB_PLATFORM_FOLDER_NAME)

        f = open(os.path.join(project_dir, "project.json"))
        project_json = json.load(f)
        f.close()
        engine_dir = os.path.join(project_json["engineDir"])
        realEngineDir = os.path.normpath(os.path.join(project_dir, engine_dir))
        publish_dir = os.path.normpath(os.path.join(project_dir, output_dir, CCPluginCompile.WEB_PLATFORM_FOLDER_NAME))

        # need to config in options of command
        buildOpt = {
                "outputFileName" : "game.min.js",
                "debug": "true" if self._is_debug_mode() else "false",
                "compilationLevel" : "advanced" if self._web_advanced else "simple",
                "sourceMapOpened" : True if self._has_sourcemap else False
                }

        if os.path.exists(publish_dir):
            shutil.rmtree(publish_dir)
        os.makedirs(publish_dir)

        # generate build.xml
        build_web.gen_buildxml(project_dir, project_json, publish_dir, buildOpt)

        outputJsPath = os.path.join(publish_dir, buildOpt["outputFileName"])
        if os.path.exists(outputJsPath) == True:
            os.remove(outputJsPath)


        # call closure compiler
        ant_root = cocos.check_environment_variable('ANT_ROOT')
        ant_path = os.path.join(ant_root, 'ant')
        self._run_cmd("%s -f %s" % (ant_path, os.path.join(publish_dir, 'build.xml')))

        # handle sourceMap
        sourceMapPath = os.path.join(publish_dir, "sourcemap")
        if os.path.exists(sourceMapPath):
            smFile = open(sourceMapPath)
            try:
                smContent = smFile.read()
            finally:
                smFile.close()

            dir_to_replace = project_dir
            if cocos.os_is_win32():
                dir_to_replace = project_dir.replace('\\', '\\\\')
            smContent = smContent.replace(dir_to_replace, os.path.relpath(project_dir, publish_dir))
            smContent = smContent.replace(realEngineDir, os.path.relpath(realEngineDir, publish_dir))
            smContent = smContent.replace('\\\\', '/')
            smContent = smContent.replace('\\', '/')
            smFile = open(sourceMapPath, "w")
            smFile.write(smContent)
            smFile.close()

        # handle project.json
        del project_json["engineDir"]
        del project_json["modules"]
        del project_json["jsList"]
        project_json_output_file = open(os.path.join(publish_dir, "project.json"), "w")
        project_json_output_file.write(json.dumps(project_json))
        project_json_output_file.close()

        # handle index.html
        indexHtmlFile = open(os.path.join(project_dir, "index.html"))
        try:
            indexContent = indexHtmlFile.read()
        finally:
            indexHtmlFile.close()
        reg1 = re.compile(r'<script\s+src\s*=\s*("|\')[^"\']*CCBoot\.js("|\')\s*><\/script>')
        indexContent = reg1.sub("", indexContent)
        mainJs = project_json.get("main") or "main.js"
        indexContent = indexContent.replace(mainJs, buildOpt["outputFileName"])
        indexHtmlOutputFile = open(os.path.join(publish_dir, "index.html"), "w")
        indexHtmlOutputFile.write(indexContent)
        indexHtmlOutputFile.close()
        
        # copy res dir
        if cfg_obj.copy_res is None:
            dst_dir = os.path.join(publish_dir, 'res')
            src_dir = os.path.join(project_dir, 'res')
            if os.path.exists(dst_dir):
                shutil.rmtree(dst_dir)
            shutil.copytree(src_dir, dst_dir)
        else:
            for cfg in cfg_obj.copy_res:
                cocos.copy_files_with_config(cfg, project_dir, publish_dir)

        # copy to the output directory if necessary
        pub_dir = os.path.normcase(publish_dir)
        out_dir = os.path.normcase(os.path.normpath(self._output_dir))
        if pub_dir != out_dir:
            cpy_cfg = {
                "from" : pub_dir,
                "to" : out_dir
            }
            cocos.copy_files_with_config(cpy_cfg, pub_dir, out_dir)

    def build_linux(self):
        if not self._platforms.is_linux_active():
            return

        #if not cocos.os_is_linux():
        #    raise cocos.CCPluginError("Please build on linux")

        project_dir = self._project.get_project_dir()
        cfg_obj = self._platforms.get_current_config()
        if cfg_obj.cmake_path is not None:
            cmakefile_dir = os.path.join(project_dir, cfg_obj.cmake_path)
        else:
            cmakefile_dir = project_dir

        # get the project name
        if cfg_obj.project_name is not None:
            self.project_name = cfg_obj.project_name
        else:
            f = open(os.path.join(cmakefile_dir, 'CMakeLists.txt'), 'r')
            regexp_set_app_name = re.compile(r'\s*set\s*\(\s*APP_NAME', re.IGNORECASE)
            for line in f.readlines():
                if regexp_set_app_name.search(line):
                    self.project_name = re.search('APP_NAME ([^\)]+)\)', line, re.IGNORECASE).group(1)
                    break
            if hasattr(self, 'project_name') == False:
	            raise cocos.CCPluginError("Couldn't find APP_NAME in CMakeLists.txt")

        if cfg_obj.build_dir is not None:
            build_dir = os.path.join(project_dir, cfg_obj.build_dir)
        else:
            build_dir = os.path.join(project_dir, 'linux-build')

        if not os.path.exists(build_dir):
            os.makedirs(build_dir)

        build_mode = 'Debug' if self._is_debug_mode() else 'Release'
        with cocos.pushd(build_dir):
            debug_state = 'ON' if self._is_debug_mode() else 'OFF'
            self._run_cmd('cmake -DCMAKE_BUILD_TYPE=%s -DDEBUG_MODE=%s %s' % (build_mode, debug_state, os.path.relpath(cmakefile_dir, build_dir)))

        with cocos.pushd(build_dir):
            self._run_cmd('make -j%s' % self._jobs)

        # move file
        output_dir = self._output_dir

        if os.path.exists(output_dir):
            shutil.rmtree(output_dir)
        os.makedirs(output_dir)

        if cfg_obj.build_result_dir is not None:
            result_dir = os.path.join(build_dir, 'bin', cfg_obj.build_result_dir, build_mode, self.project_name)
        else:
            result_dir = os.path.join(build_dir, 'bin', build_mode, self.project_name)
        cocos.copy_files_in_dir(result_dir, output_dir)

        self.run_root = output_dir

        if self._no_res:
            res_dir = os.path.join(output_dir, "Resources")
            self._remove_res(res_dir)

        if self._project._is_script_project() and self._compile_script:
            cocos.Logging.warning(MultiLanguage.get_string('COMPILE_WARNING_NOT_SUPPORT_COMPILE_SCRIPT'))

        cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILD_SUCCEED'))

    def build_metro(self):
        if not self._platforms.is_metro_active():
            return

        metro_projectdir = self._platforms.project_path()
        output_dir = self._output_dir

        cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILDING'))

        # get the solution file & project name
        cfg_obj = self._platforms.get_current_config()
        if cfg_obj.sln_file is not None:
            sln_name = cfg_obj.sln_file
            if cfg_obj.project_name is None:
                raise cocos.CCPluginError(MultiLanguage.get_string('COMPILE_ERROR_CFG_NOT_FOUND_FMT',
                                                                   (cocos_project.Win32Config.KEY_PROJECT_NAME,
                                                                    cocos_project.Win32Config.KEY_SLN_FILE,
                                                                    cocos_project.Project.CONFIG)),
                                          cocos.CCPluginError.ERROR_WRONG_CONFIG)
            else:
                name = cfg_obj.project_name
        else:
            name, sln_name = self.checkFileByExtention(".sln", metro_projectdir)
            if not sln_name:
                message = MultiLanguage.get_string('COMPILE_ERROR_SLN_NOT_FOUND')
                raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PATH_NOT_FOUND)
            name = "%s.Windows" % name

        # build the project
        self.project_name = name
        projectPath = os.path.join(metro_projectdir, sln_name)
        build_mode = 'Debug' if self._is_debug_mode() else 'Release'
        self.build_vs_project(projectPath, self.project_name, build_mode, self.vs_version)

    def _get_build_cfg(self):
        build_cfg_dir = self._build_cfg_path()
        build_cfg = os.path.join(build_cfg_dir, CCPluginCompile.BUILD_CONFIG_FILE)
        if not os.path.exists(build_cfg):
            message = MultiLanguage.get_string('COMPILE_ERROR_FILE_NOT_FOUND_FMT', build_cfg)
            raise cocos.CCPluginError(message, cocos.CCPluginError.ERROR_PATH_NOT_FOUND)
        f = open(build_cfg)
        return json.load(f)

    def _copy_resources(self, dst_path):
        data = self._get_build_cfg()

        if data.has_key(CCPluginCompile.CFG_KEY_MUST_COPY_RESOURCES):
            if self._no_res:
                fileList = data[CCPluginCompile.CFG_KEY_MUST_COPY_RESOURCES]
            else:
                fileList = data[CCPluginCompile.CFG_KEY_COPY_RESOURCES] + data[CCPluginCompile.CFG_KEY_MUST_COPY_RESOURCES]
        else:
            fileList = data[CCPluginCompile.CFG_KEY_COPY_RESOURCES]

        for cfg in fileList:
            cocos.copy_files_with_config(cfg, self._build_cfg_path(), dst_path)

    def checkFileByExtention(self, ext, path):
        filelist = os.listdir(path)
        for fullname in filelist:
            name, extention = os.path.splitext(fullname)
            if extention == ext:
                return name, fullname
        return (None, None)

    def run(self, argv, dependencies):
        self.parse_args(argv)
        cocos.Logging.info(MultiLanguage.get_string('COMPILE_INFO_BUILD_MODE_FMT', self._mode))
        self._update_build_cfg()

        target_platform = self._platforms.get_current_platform()
        args_build_copy = self._custom_step_args.copy()

        language = self._project.get_language()
        action_str = 'compile_%s' % language
        target_str = 'compile_for_%s' % target_platform
        cocos.DataStatistic.stat_event('compile', action_str, target_str)

        # invoke the custom step: pre-build
        self._project.invoke_custom_step_script(cocos_project.Project.CUSTOM_STEP_PRE_BUILD, target_platform, args_build_copy)

        self.build_android()
        self.build_ios()
        self.build_mac()
        self.build_win32()
        self.build_web()
        self.build_linux()
        self.build_metro()

        # invoke the custom step: post-build
        self._project.invoke_custom_step_script(cocos_project.Project.CUSTOM_STEP_POST_BUILD, target_platform, args_build_copy)

        if len(self.end_warning) > 0:
            cocos.Logging.warning(self.end_warning)
