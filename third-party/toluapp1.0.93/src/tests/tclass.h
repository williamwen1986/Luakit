#include <stdio.h>

class Tst_Dummy
{
};

class Tst_A
{
public:
  static Tst_A* last;
  Tst_A () {last = this;}
  virtual ~Tst_A() {}
  virtual const char* a () { return "A"; }
  class Tst_AA
  {
 public:
    Tst_AA () {}
    virtual ~Tst_AA () { }
    const char* aa () { return "AA"; }
  };
  class Tst_BB : public Tst_AA
  {
 public:
    Tst_BB () {}
    virtual ~Tst_BB () {}
    Tst_AA* Base () { return this; }
    const char* classname () const {
      return "Tst_BB";
    }
  };
};

class Tst_B : public Tst_A
{
public:
  static Tst_B* last;
  Tst_B () {last = this;}
  virtual ~Tst_B () { }
  virtual const char* b () { return "B"; }
};

class Tst_C : public Tst_B
{
  int i;
public:
  static Tst_C* last;
  Tst_C (int n) : i(n) {last = this;}
 virtual ~Tst_C () { printf("deleting C: %d\n",i); }
  virtual const char* c () { return "C"; }
};

inline Tst_A::Tst_AA* Tst_create_aa ()
{
  return new Tst_A::Tst_AA();
}

inline bool Tst_is_aa (Tst_A::Tst_AA* obj)
{
  return true;
}

class D // private destructor
{
  ~D () {}
public:
  D () {}
};
