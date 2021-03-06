/*!
 * Función non_zero:
 * calcula un vector booleano que registra los elementos diferentes de cero
 * del vector de entrada x.
 * 
 * @param x puntero al vector a registrarle los elementos diferentes de cero.
 * @param tm tamaño del vector x.
 * @param nz_x puntero al vector donde se registran los elementos diferentes de ccero de x.
 * 
 */

void non_zero(double *x, int tm, double *nz_x)
{
	for(int i=0;i<tm;i++)
	{
		if(x[i]!=0)
			nz_x[i]=1;
		else
			nz_x[i]=0;
	}
}
