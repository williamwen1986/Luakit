#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs the test with xvfb on linux. Runs the test normally on other platforms.

For simplicity in gyp targets, this script just runs the test normal on
non-linux platforms.
"""

import os
import platform
import signal
import subprocess
import sys

import test_env


def kill(pid):
  """Kills a process and traps exception if the process doesn't exist anymore.
  """
  # If the process doesn't exist, it raises an exception that we can ignore.
  try:
    os.kill(pid, signal.SIGKILL)
  except OSError:
    pass


def get_xvfb_path(server_dir):
  """Figures out which X server to use."""
  xvfb_path = os.path.join(server_dir, 'Xvfb.' + platform.architecture()[0])
  if not os.path.exists(xvfb_path):
    xvfb_path = os.path.join(server_dir, 'Xvfb')
  if not os.path.exists(xvfb_path):
    print >> sys.stderr, (
        'No Xvfb found in designated server path: %s' % server_dir)
    raise Exception('No virtual server')
  return xvfb_path


def start_xvfb(xvfb_path, display):
  """Starts a virtual X server that we run the tests in.

  This makes it so we can run the tests even if we didn't start the tests from
  an X session.

  Args:
    xvfb_path: Path to Xvfb.
  """
  proc = subprocess.Popen(
      [xvfb_path, display, '-screen', '0', '1024x768x24', '-ac'],
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT)
  return proc.pid


def wait_for_xvfb(xdisplaycheck, env):
  """Waits for xvfb to be fully initialized by using xdisplaycheck."""
  try:
    subprocess.check_call(
        [xdisplaycheck],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        env=env)
  except OSError:
    print >> sys.stderr, 'Failed to load %s with cwd=%s' % (
        xdisplaycheck, os.getcwd())
    return False
  except subprocess.CalledProcessError:
    print >> sys.stderr, (
        'Xvfb failed to load properly while trying to run %s' % xdisplaycheck)
    return False
  return True


def run_executable(cmd, build_dir, env):
  """Runs an executable within a xvfb buffer on linux or normally on other
  platforms.

  Requires that both xvfb and icewm are installed on linux.
  """
  pid = None
  xvfb = 'Xvfb'
  try:
    if sys.platform == 'linux2':
      # Defaults to X display 9.
      display = ':9'
      pid = start_xvfb(xvfb, display)
      env['DISPLAY'] = display
      if not wait_for_xvfb(os.path.join(build_dir, 'xdisplaycheck'), env):
        return 3
      # Some ChromeOS tests need a window manager. Technically, it could be
      # another script but that would be overkill.
      subprocess.Popen(
          'icewm', stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
    return test_env.run_executable(cmd, env)
  finally:
    if pid:
      kill(pid)


def main():
  if len(sys.argv) < 3:
    print >> sys.stderr, (
        'Usage: xvfb.py [path to build_dir] [command args...]')
    return 2
  return run_executable(sys.argv[2:], sys.argv[1], os.environ.copy())


if __name__ == "__main__":
  sys.exit(main())
