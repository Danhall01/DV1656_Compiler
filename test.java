public class A {
	public static void main(String[] a) {
		System.out.println(new Bar().foo(true, 5));
	}
}

class Bar {
	public int foo(boolean bool, int integer) {
		int aux;
		boolean aux2;
		int[] aux3;
		aux3 = new int[10];
		aux = 1;
		aux2 = true;
        aux = aux2;
		aux = 1*3+aux3.length;

		while (aux == 100)
		{
			if (false == true)
				aux = 2;
			else
				aux = 5;
		}

		if ( aux==(3+1*5) )
		{
			while (true)
			{
				aux3[5] = aux;
				aux = aux3[2] / (5 - (4 + aux));
				aux3[aux3.length] = 5*7;
				System.out.println(2*aux);
			}
		}

		return aux;
	}
}