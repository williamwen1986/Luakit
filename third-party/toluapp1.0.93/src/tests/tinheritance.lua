-- type convertion tests
local a = A:new()
local b = B:new()
local c = C:new()
local d = D:new()

assert(b:name()=="B")
assert(b:aname()=="A")
assert(c:name()=="C")
assert(c:aname()=="A")
assert(d:name()=="D")
assert(d:aname()=="A")

print("Inheritance test OK")
