#pragma once

#include <string>

class BusinessMainDelegate {
public:
  BusinessMainDelegate() {}
  
  virtual ~BusinessMainDelegate() {
  
  }
  
  // Tells the embedder that the absolute basic startup has been done, i.e.
  // it's now safe to create singletons and check the command line. Return true
  // if the process should exit afterwards, and if so, |exit_code| should be
  // set. This is the place for embedder to do the things that must happen at
  // the start. Most of its startup code should be in the methods below.
  virtual bool BasicStartupComplete(int* exit_code);
  
  // This is where the embedder puts all of its startup code that needs to run
  // before the sandbox is engaged.
  virtual void PreSandboxStartup();

  // This is where the embedder can add startup code to run after the sandbox
  // has been initialized.
  virtual void SandboxInitialized() {}

  virtual void PostCreateThreads();
  
  // Called right before the process exits.
  virtual void ProcessExiting();
};
