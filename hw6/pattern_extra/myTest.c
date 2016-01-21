int zero(){ write("0\n"); return 0;}
int one(){ write("1\n"); return 1;}
int two(){ write("2\n"); return 2;}
int three(){ write("3\n"); return 3;}

int MAIN(){
  if(one() || zero()){
    write("correct: 1\n");
  }else{
    write("wrongn\n");
  }
  if(zero() || one()){
    write("correct: 0 1\n");
  }else{
    write("wrongn\n");
  }

  if(one() && zero()){
    write("wrongn\n");
  }else{
    write("correct: 1 0\n");
  }
  if(zero() && one()){
    write("wrongn\n");
  }else{
    write("correct: 0\n");
  }
  if(one() && two()){
    write("correct: 1 2\n");
  }else{
    write("wrongn\n");
  }
  if(one() || (two() / zero())){
    write("correct: 1\n");
  }
}
