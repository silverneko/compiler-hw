int f(int a){
	a = a+1;
	a = a+a;
	return a;
}
int main(){
	float f1;
	int i;
	i = f();
	i = f(1,2);
	i = f(1);
	return 0;
}
