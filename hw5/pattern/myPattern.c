int MAIN(){
  int a;
  int b;
  int c;
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
  if(a < b)
    write("a < b\n");
  else if(a > b)
    write("a > b\n");
  else
    write("a == b\n");

  write(a < b);
  write("while\n");
  while(a < b) a = a + 1;
  /* This may break
  while(a = b){
  }
  */
  return 0;
}
