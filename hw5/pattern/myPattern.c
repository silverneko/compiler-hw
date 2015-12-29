int veryLargeArray[100000];

int f(){
  return 123;
}

float g(){
  return 123.456;
  return 42689.2;
}

void h(){
  return ;
}

int MAIN(){
  int a;
  int b;
  int c;
  float f;
  {
    int a, b, c;
    {
      int a, b, c;
    }
  }
  veryLargeArray[10000] = 12;
  if(a = 0){
    write("wrong\n");
  }else{
    write("correct\n");
  }
  if(a = 1 - 3){
    write("correct\n");
  }else{
    write("wrong\n");
  }
  if(a = 1.0 / 3.0){
    write("wrong\n");
  }else{
    write("correct\n");
  }
  if(f = 1.0 / 3.0){
    write("wrong\n");
  }else{
    write("correct\n");
  }
  a = 10000000;
  write(a);
  write("\n");
  a = 1 + 3;
  write(a);
  write("\n");
  a = 10;
  b = 20;
  c = 30;
  write(a+b*c);
  write("\n");
  a = 10;
  b = a;
  if(a < b){
    write("a < b\n");
  }else if(a > b){
    write("a > b\n");
  }else{
    write("a == b\n");
  }
  a = 14;
  if(1.0 < 2.0){
    int d, a;
    write("A_A\n");
    a = 12;
    write(a);
    write("\n");
  }
  write(a);
  write("\n");

  write(a < b);
  write("while\n");
  a = 0;
  b = 10;
  while(a < b){
    int blob[10000];
    write(a);
    write("\n");
    a = a + 1;
  }
  /* This may break
  while(a = b){
  }
  */
  a = (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + (a + a))))))))))))))))))))))))))))));
  return 0;
}
