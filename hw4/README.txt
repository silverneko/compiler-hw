For a function call, we've also checked if it is really a function name.
i.e.
```
int main(){
  int f;
  f();
}
```
This will report `ID f is not function name.`.
