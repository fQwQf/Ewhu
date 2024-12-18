# Ewhu
## Build
### windows
```bash
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
cmake --build . -j4
```
## bench

```bash
valgrind --tool=callgrind ./Ewhu -b [script]
```
## Count line
```bash
(Get-ChildItem -Recurse -Include *.h, *.cpp | Where-Object { $_.FullName -notmatch '\\(rapidjson|build)\\' } | Get-Content | Measure-Object -Line).Lines
```
now: 2730
## Usage
### Paradigms

* Object-Oriented Programming (OOP) 
* Explicit Typed
* Statically Typed

### Comment
```
# this is a comment
```

### Operator
```
a = 1 + 1
b = 3 - 1
c = 2 * 1
d = 4 / 2
e = 5 // 3
f = 7 % 3
g = 4 ^ (0.5)

xor
or
not
and
```


### Type
```cpp
bool
int
fraction
unreal
unreal_frac
list = [a,b,c]
```

### Function
```cpp
func_name(arg1,arg2) = ();
func_name(arg1,arg2);
```

### Control Flow
```cpp
(a)?(b):(c);
if(a)then(b);
if(a)then(b)else(c);
while(a)do(b);
```
