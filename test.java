public class A {
	public static void main(String[] a) {
		System.out.println(new Bar().foo());
	}
}

class Bar {
	public int foo() {
		int aux;
		boolean aux2;
		aux = 1;
		aux2 = true;
        aux = aux2;
		aux = 1*3+aux;
		return aux;
	}
}