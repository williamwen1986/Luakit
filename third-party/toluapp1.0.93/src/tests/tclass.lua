-- type convertion tests
assert(tolua.type(A.last) == 'Tst_A') -- first time the object is mapped
assert(tolua.type(B.last) == 'Tst_B') -- type convertion to specialized type
assert(tolua.type(A.last) == 'Tst_B') -- no convertion: obj already mapped as B


local a = A:new()
assert(tolua.type(A.last) == 'Tst_A') -- no type convertion: same type
local b = B:new()
assert(tolua.type(A.last) == 'Tst_B') -- no convertion: obj already mapped as B
local c = luaC:new(0)
assert(tolua.type(A.last) == 'Tst_C') -- no convertion: obj already mapped as C
assert(tolua.type(luaC.last) == 'Tst_C')
local aa = A.AA:new()
local bb = A.BB:new()
local xx = create_aa()

-- casting test
local base = bb:Base();
local derived = tolua.cast(base,"Tst_A::Tst_BB");
assert(derived:classname()=="Tst_BB")

-- method calling tests
assert(a:a() == 'A')
assert(b:a() == 'A')
assert(b:b() == 'B')
assert(c:a() == 'A')
assert(c:b() == 'B')
assert(c:c() == 'C')
assert(aa:aa() == 'AA')
assert(bb:aa() == bb:Base():aa())
assert(xx:aa() == 'AA')
assert(is_aa(bb) == true)

-- test ownershipping handling
-- should delete objects: 6 7 8 9 10 (it may vary!)
remark = [[
local set = {}
for i=1,10 do
 local c = luaC:new(i)
  tolua.takeownership(c)
  set[i] = c
end

for i=1,5 do
 tolua.releaseownership(set[i])
end
--]]

print("Class test OK")
