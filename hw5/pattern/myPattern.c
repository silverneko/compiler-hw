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
  veryLargeArray[10000] = 12;
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
    return 1;
  }

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
