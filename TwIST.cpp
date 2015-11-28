void TwIST(double *y,const object *A, double tau,double *&x,int arguments, ...)
{
	
	int tmy = 1024; //Static size for example :D
	int tm = 4096;

    int stopCriterion = 1;
    float criterion = 0;
    double tolA = 0.01;
    bool debias = false;
    int maxiter = 10000;
    int maxiter_debias = 200;
    int miniter = 5;
    int miniter_debias = 5;
    int init = 0;
    int enforceMonotone = 1;
    bool compute_mse = false;
    int plot_ISNR = 0;
    bool verbose = true;
    double alpha = 0;
    double beta = 0;
    
    float prev_f = 0;
    float f = 0;
    
    int sparse = 1;
    double tolD = 0.001;
    
    //variable phi_l1 and psi_ok 
    double phi_l1 = 0;
    double psi_ok = 0;
    
    //Default eigenvalues
    double lam1 = 1e-4;
    double lamN = 1;
    
    //constants ans internal variables
    double for_ever = 1;
    
    //maj_max_sv: majorizer for the maximum singular value of operator A
    int max_svd = 1;

    //Set the defaults for outputs that may not be computed
    int debias_start = 0;
    double x_debias = [];
    double mses = [];

    char *str=0;
    int i;
    

    

    va_list args;
    va_start(args, arguments);
    
    /*-----------------------
            Optional parameters
    --------------------------*/
    while (true)
    {
        str = va_arg (args,char *);
        if ( strcmp(str,"LAMBDA")==0){
            lam1 = va_arg (args, double);
            cout << "Lambda: "<<lam1 << '\n';            
        }
        else if (strcmp(str,"ALPHA")==0){
            alpha = va_arg (args, double);
            cout <<"Alpha: "<< alpha << '\n';
        }
        else if (strcmp(str,"BETA")==0){
            beta = va_arg (args, double);
            cout <<"Beta: "<< beta << '\n';
        }
        //-------
        //Add Psi and Phi Varargin (Check dependencies) ----------------------------------------------------Check
        //-------
        else if ( strcmp(str,"STOPCRITERION")==0){
            stopCriterion = va_arg (args, int);
            cout << "Stop criterion: "<<stopCriterion << '\n';
        }
        else if(strcmp(str,"TOLERANCEA")==0){
            tolA = va_arg (args, double);
            cout << "Tolerancia: "<<tolA << '\n';
        }
        else if(strcmp(str,"TOLERANCED")==0){
            tolD = va_arg (args, double);
            cout << "Toleranced: "<<tolD << '\n';
        }
        else if(strcmp(str,"DEBIAS")==0){
            debias = va_arg (args, bool);
            cout << "Debias: "<<debias << '\n';
        }
        else if(strcmp(str,"MAXITERA")==0){
            maxiter = va_arg (args, int);
            cout << "Maxitera: "<<maxiter << '\n';
        }
        else if(strcmp(str,"MAXITERD")==0){
            maxiter_debias = va_arg (args, int);
            cout <<"Maxiter debias: "<< maxiter_debias << '\n';
        }
        else if(strcmp(str,"MINITERA")==0){
            miniter = va_arg (args, int);
            cout <<"Mintera: "<< miniter << '\n';
        }
        else if(strcmp(str, "MINITERD")==0){
            miniter_debias = va_arg (args, int);
            cout <<"Miniter debias: "<< miniter_debias << '\n';
        }
        else if(strcmp(str,"INITIALIZATION")==0){
            init=va_arg (args, int);
            cout<<"Init: "<<init<<endl;
            //Inizialization have an "If" ---> Check O_O ------------------------------------------------Check
        }
        else if(strcmp(str,"MONOTONE")==0){
            enforceMonotone = va_arg (args, int);
            cout <<"EnforceMonotone: "<< enforceMonotone << '\n';
        }
        else if(strcmp(str,"SPARSE")==0){
            sparse = va_arg (args, int);
            cout <<"Sparse: "<< sparse << '\n';
        }
        else if(strcmp(str,"TRUE_X")==0){
            compute_mse = true;
            va_arg (args, bool);
        }
        else if(strcmp(str,"AT")==0){
            AT = va_arg (args, int);
            cout <<"AT: "<< AT << '\n';
        }        
        else if(strcmp(str,"VERBOSE")==0){
            verbose = va_arg (args, int);
            cout <<"Verbose: "<< verbose << '\n';
        }
        else{
            break;
        }
    }
    va_end (args);
    
    //Others 
    
    static float *nz_x=new float[tm];
    static float *mask=new float[tm];
    static float *nz_x_prev=new float[tm];
    
    static float *resid=new float[tmy];
    
    static float *temp0=new float[tm];
    static float *temp=new float[tm];
    static float *temp1=new float[tm];
    static float *temp2=new float[tm];
    static float *temp3=new float[tm];
    static float *temp4=new float[tmy];

	//static float *objective=new float[100];
	float objective;
    float num_changes_active;
    
    //TwIST Parameters
    double rho0;
    rho0 = (1 - lam1/lamN)/(1 + lam1/lamN);
    if (alpha == 0)
    {
        alpha = 2/(1+sqrt(1-rho0^2));
    }
    if (beta == 0)
    {
        beta  = (alpha*2)/(lam1+lamN);
    }

    
    switch(init)
    {
    case 0: 
    {
        x = new float[tm];

        for(i=0;i<tm;i++){
            x[i]=0.0;
        }
        break;
    };
    default: cout<<"Unknown Initialization option"<<endl;break;
	}
    
    
    float max_tau;
    max_tau = 0.0;

	 //Define the indicator vector or matrix of nonzeros in x
	float num_nz_x;
	
	non_zero(x,tm,nz_x); //Extract the valuez nonzero in a vector
	num_nz_x = sum(nz_x,tm); //Sum the values in a vector
	 
	// Compute and store initial value of the objective function
	temp = A(x);
	vector_res(y, temp, tmy, resid);
	float c;
	c = vector_prod(resid,resid,tmy);
	prev_f = 0.5*c + tau*phi_function(x,tm); //499
	
	//Start clock	
	//t0 = clock();
	//times(1) = cputime - t0;
	objective[0] = prev_f;
	
	int cont_outer = 1;
	int iter = 1;
	
	// variables controling first and second order iterations
	int IST_iters = 0;
	int TwIST_iters = 0;

	// initialize
	static float *xm2=new float[tm];
    static float *xm1=new float[tm];
    float c;
	xm2=x;
	xm1=x;	
	
	
	/*--------------------------------------------------------------
		TwIST iterations
	-------------------------------------------------------------*/
	while (cont_outer)
	{
    // gradient
    grad = AT(resid);
		while (for_ever)
		{
			prod_c_v(grad, (1/max_svd), tm, temp0);
			vector_sum(xm1, temp0, tm, temp0);
			psi_function(temp0, tau/max_svd, tm, x);
			
			if ((IST_iters >= 2) || ( TwIST_iters != 0))
			{
				if (sparse)
				{	
					non_zero(x, tm, mask);
					hadamard(xm1, mask, tm, xm1);
					hadamard(xm2, mask, tm, xm2);
				}
			
				// two-step iteration
				prod_c_V(xm1,(alpha-beta), tm, temp1);
				prod_c_V(xm2,(1-alpha), tm, temp2);
				prod_c_V(x, beta, tm, temp3);
				vector_sum(temp1,temp2, tm, xm2);
				vector_sum(xm2,temp3, tmy, xm2);
            
				// compute residual
				temp4 = A(xm2)
				vector_res(y, temp4, tmy, resid);
				
				float c;
				c = vector_prod(resid,resid,tmy);
				f = 0.5*c + tau*phi_function(xm2,tm);
				
				if ((f > prev_f) && (enforceMonotone))
				{
					TwIST_iters = 0;  /*! do a IST iteration if monotonocity fails*/
				}
				else
				{
					TwIST_iters = TwIST_iters+1; // TwIST iterations
					IST_iters = 0;
					x = xm2;
					if (TwIST_iters % 10000) == 0
					{
						max_svd = 0.9*max_svd;
					}
					break;  // break loop while
				}	
			}	
			else
			{
				temp4 = A(x)
				vector_res(y, temp4, tmy, resid);
				
				c = vector_prod(resid,resid,tmy);
				f = 0.5*c + tau*phi_function(x,tm);
				
				if (f > prev_f)
				{
					max_svd = 2*max_svd;
					IST_iters = 0;
					TwIST_iters = 0;
				}
				else
				{
					TwIST_iters = TwIST_iters + 1;
					break;
				}
			}
		}
		//while
		xm2 = xm1;
		xm1 = x;        
       
		//update the number of nonzero components and its variation
		nz_x_prev = nz_x;
		non_zero(x, tm, nz_x);
		num_nz_x = sum(nz_x, tm);
		diff(nz_x,nz_x_prev, tm, temp);
		num_changes_active = sum(temp,tm);

		// take no less than miniter and no more than maxiter iterations
		switch (stopCriterion)
		{
			case 0:
			{
				/* compute the stopping criterion based on the change
				of the number of non-zero components of the estimate*/
				criterion =  num_changes_active;
			}
			case 1:
			{
				/* compute the stopping criterion based on the relative
				variation of the objective function.*/
				criterion = abs(f-prev_f)/prev_f;
			}
			case 2:
			{
				/* compute the stopping criterion based on the relative
				% variation of the estimate.
				criterion = (norm(x(:)-xm1(:))/norm(x(:)));*/
			}
			case 3:
			{
				// continue if not yet reached target value tolA
				criterion = f;
			}	
			default:
			{
				printf("Stop Criterion Error");
			}
		}
		cont_outer = ((iter <= maxiter) && (criterion > tolA));
		if (iter <= miniter)
		{
			cont_outer = 1;
		}
	
		iter = iter + 1;
		prev_f = f;
		objective = f;
		//times(iter) = cputime - t0;    
	
	}
	printf("\nFinished the main algorithm!\nResults:\n");
    printf("||A x - y ||_2^2 = %8.20f\n",vector_prod(resid,resid,tmy));
	//cout<<"||x||_1 = "<<mysum(x,1.0,tm,0)<<endl;
    printf("Objective function = %10.3f\n",f);
    //printf("Number of non-zero components = %2.0f\n",num_nz_x);
}
