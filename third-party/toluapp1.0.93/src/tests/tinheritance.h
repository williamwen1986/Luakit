#include <stdio.h>

class A
{
public:
	A () {}
  virtual ~A() {}
  virtual const char* aname () { return "A"; }
	virtual const char* name () { return "A"; } 
};

class B : public A
{
public:
	B () {}
  virtual ~B() {}
	virtual const char* name () { return "B"; } 
};

class C : public B
{
public:
	C () {}
  virtual ~C() {}
	virtual const char* name () { return "C"; } 
};

class D : public C
{
public:
	D () {}
  virtual ~D() {}
	virtual const char* name () { return "D"; } 
};

