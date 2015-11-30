/*!
 * Función AT:
 * La función AT realiza la  multiplicación de un vector x 
 * con una matriz R.
 * 
 * @param x puntero al vector a registrarle los elementos diferentes de cero.
 * @param R Puntero a la Matriz 
 * @param m el tamaño de las filas de R
 * @param n el tamaño de las columnas de R
 * @param nz_x puntero al vector resultante
 * 
 */

void AT(float *y, matrix_A *&matrixA, int m, int n, float *result)
{
	float var;
	int p;
	int cont = 0;
	for(int i = 0; i < n ; i++)
	{
		p = 0;
		var = 0;
		for(int j = 0; j < m; j++)
		{
			var = var + matrixA[j][i]*y[p];
			p++;
		}
		result[cont] = var;
		cont++;
	}
}
