#include <math.h>
#include <stdio.h>

extern "C" void
ScaLBL_D3Q7_AAodd_Poisson_ElectricPotential(int *neighborList, int *Map,
                                            double *dist, double *Psi,
                                            int start, int finish, int Np) {
    int n;
    double psi; //electric potential
    double fq;
    int nread;
    int idx;

    for (n = start; n < finish; n++) {

        // q=0
        fq = dist[n];
        psi = fq;

        // q=1
        nread = neighborList[n];
        fq = dist[nread];
        psi += fq;

        // q=2
        nread = neighborList[n + Np];
        fq = dist[nread];
        psi += fq;

        // q=3
        nread = neighborList[n + 2 * Np];
        fq = dist[nread];
        psi += fq;

        // q = 4
        nread = neighborList[n + 3 * Np];
        fq = dist[nread];
        psi += fq;

        // q=5
        nread = neighborList[n + 4 * Np];
        fq = dist[nread];
        psi += fq;

        // q = 6
        nread = neighborList[n + 5 * Np];
        fq = dist[nread];
        psi += fq;

        idx = Map[n];
        Psi[idx] = psi;
    }
}

extern "C" void ScaLBL_D3Q7_AAeven_Poisson_ElectricPotential(
    int *Map, double *dist, double *Psi, int start, int finish, int Np) {
    int n;
    double psi; //electric potential
    double fq;
    int idx;

    for (n = start; n < finish; n++) {

        // q=0
        fq = dist[n];
        psi = fq;

        // q=1
        fq = dist[2 * Np + n];
        psi += fq;

        // q=2
        fq = dist[1 * Np + n];
        psi += fq;

        // q=3
        fq = dist[4 * Np + n];
        psi += fq;

        // q=4
        fq = dist[3 * Np + n];
        psi += fq;

        // q=5
        fq = dist[6 * Np + n];
        psi += fq;

        // q=6
        fq = dist[5 * Np + n];
        psi += fq;

        idx = Map[n];
        Psi[idx] = psi;
    }
}

extern "C" void ScaLBL_D3Q7_AAodd_Poisson(int *neighborList, int *Map,
                                          double *dist, double *Den_charge,
                                          double *Psi, double *ElectricField,
                                          double tau, double epsilon_LB, bool UseSlippingVelBC,
                                          int start, int finish, int Np) {
    int n;
    double psi;        //electric potential
    double Ex, Ey, Ez; //electric field
    double rho_e, rho_p;      //local charge density
    double f0, f1, f2, f3, f4, f5, f6;
    int nr1, nr2, nr3, nr4, nr5, nr6;
    double rlx = 1.0 / tau;
    int idx;
    
    // Universal constant
    double kb = 1.38e-23;                   //Boltzmann constant;unit [J/K]
    double electron_charge = 1.6e-19;       //electron charge;unit [C]
    double T = 300.0;                       //temperature; unit [K]
    double Vt = electron_charge / (kb * T); // 1 / thermal voltage; unit [Vy]

    for (n = start; n < finish; n++) {

        //Load data
        //When Helmholtz-Smoluchowski slipping velocity BC is used, the bulk fluid is considered as electroneutral
        //and thus the net space charge density is zero. 
        rho_e = (UseSlippingVelBC==1) ? 0.0 : Den_charge[n] / epsilon_LB;
        idx = Map[n];
        psi = Psi[idx];
        
        /* Compute H30+  OH-  charge density from Poisson Boltzmann statistics */
        rho_p = 1.04e-7 * (exp(psi*Vt) - exp((-1.0)*psi*Vt));
        rho_e += rho_p;

        // q=0
        f0 = dist[n];
        // q=1
        nr1 = neighborList[n]; // neighbor 2 ( > 10Np => odd part of dist)
        f1 = dist[nr1];        // reading the f1 data into register fq

        nr2 = neighborList[n + Np]; // neighbor 1 ( < 10Np => even part of dist)
        f2 = dist[nr2];             // reading the f2 data into register fq

        // q=3
        nr3 = neighborList[n + 2 * Np]; // neighbor 4
        f3 = dist[nr3];

        // q = 4
        nr4 = neighborList[n + 3 * Np]; // neighbor 3
        f4 = dist[nr4];

        // q=5
        nr5 = neighborList[n + 4 * Np];
        f5 = dist[nr5];

        // q = 6
        nr6 = neighborList[n + 5 * Np];
        f6 = dist[nr6];

        Ex = (f1 - f2) * rlx *
             4.0; //NOTE the unit of electric field here is V/lu
        Ey = (f3 - f4) * rlx *
             4.0; //factor 4.0 is D3Q7 lattice squared speed of sound
        Ez = (f5 - f6) * rlx * 4.0;
        ElectricField[n + 0 * Np] = Ex;
        ElectricField[n + 1 * Np] = Ey;
        ElectricField[n + 2 * Np] = Ez;

        // q = 0
        dist[n] = f0 * (1.0 - rlx) + 0.25 * (rlx * psi + rho_e);

        // q = 1
        dist[nr2] = f1 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 2
        dist[nr1] = f2 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 3
        dist[nr4] = f3 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 4
        dist[nr3] = f4 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 5
        dist[nr6] = f5 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 6
        dist[nr5] = f6 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);
        //........................................................................
    }
}

extern "C" void ScaLBL_D3Q7_AAeven_Poisson(int *Map, double *dist,
                                           double *Den_charge, double *Psi,
                                           double *ElectricField, double tau,
                                           double epsilon_LB, bool UseSlippingVelBC,
                                           int start, int finish, int Np) {
    int n;
    double psi;        //electric potential
    double Ex, Ey, Ez; //electric field
    double rho_e, rho_p;      //local charge density
    double f0, f1, f2, f3, f4, f5, f6;
    double rlx = 1.0 / tau;
    int idx;
    
    // Universal constant
    double kb = 1.38e-23;                   //Boltzmann constant;unit [J/K]
    double electron_charge = 1.6e-19;       //electron charge;unit [C]
    double T = 300.0;                       //temperature; unit [K]
    double Vt = electron_charge / (kb * T); // 1 / thermal voltage; unit [Vy]

    for (n = start; n < finish; n++) {

        //Load data
        //When Helmholtz-Smoluchowski slipping velocity BC is used, the bulk fluid is considered as electroneutral
        //and thus the net space charge density is zero. 
        rho_e = (UseSlippingVelBC==1) ? 0.0 : Den_charge[n] / epsilon_LB;
        idx = Map[n];
        psi = Psi[idx];
        
        /* Compute H30+  OH-  charge density from Poisson Boltzmann statistics */
        rho_p = 1.04e-7 * (exp(psi*Vt) - exp((-1.0)*psi*Vt));
        rho_e += rho_p;

        f0 = dist[n];
        f1 = dist[2 * Np + n];
        f2 = dist[1 * Np + n];
        f3 = dist[4 * Np + n];
        f4 = dist[3 * Np + n];
        f5 = dist[6 * Np + n];
        f6 = dist[5 * Np + n];

        Ex = (f1 - f2) * rlx *
             4.0; //NOTE the unit of electric field here is V/lu
        Ey = (f3 - f4) * rlx *
             4.0; //factor 4.0 is D3Q7 lattice squared speed of sound
        Ez = (f5 - f6) * rlx * 4.0;
        ElectricField[n + 0 * Np] = Ex;
        ElectricField[n + 1 * Np] = Ey;
        ElectricField[n + 2 * Np] = Ez;

        // q = 0
        dist[n] = f0 * (1.0 - rlx) + 0.25 * (rlx * psi + rho_e);

        // q = 1
        dist[1 * Np + n] = f1 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 2
        dist[2 * Np + n] = f2 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 3
        dist[3 * Np + n] = f3 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 4
        dist[4 * Np + n] = f4 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 5
        dist[5 * Np + n] = f5 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);

        // q = 6
        dist[6 * Np + n] = f6 * (1.0 - rlx) + 0.125 * (rlx * psi + rho_e);
        //........................................................................
    }
}

extern "C" void ScaLBL_D3Q7_Poisson_Init(int *Map, double *dist, double *Psi,
                                         int start, int finish, int Np) {
    int n;
    int ijk;
    for (n = start; n < finish; n++) {
        ijk = Map[n];
        dist[0 * Np + n] = 0.25 * Psi[ijk];
        dist[1 * Np + n] = 0.125 * Psi[ijk];
        dist[2 * Np + n] = 0.125 * Psi[ijk];
        dist[3 * Np + n] = 0.125 * Psi[ijk];
        dist[4 * Np + n] = 0.125 * Psi[ijk];
        dist[5 * Np + n] = 0.125 * Psi[ijk];
        dist[6 * Np + n] = 0.125 * Psi[ijk];
    }
}

extern "C" void ScaLBL_D3Q7_PoissonResidualError(
    int *neighborList, int *Map, double *ResidualError, double *Psi,
    double *Den_charge, double epsilon_LB, int strideY, int strideZ, int start,
    int finish) {

    int n, nn, ijk;
    double psi;   //electric potential
    double rho_e; //local charge density
    // neighbors of electric potential psi
    double m1, m2, m4, m6, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18;
    double m3, m5, m7;
    double psi_Laplacian;
    double residual_error;

    for (n = start; n < finish; n++) {

        //Load data
        rho_e = Den_charge[n];
        ijk = Map[n];
        psi = Psi[ijk];

        //					COMPUTE THE COLOR GRADIENT
        //........................................................................
        //.................Read Phase Indicator Values............................
        //........................................................................
        nn = ijk - 1; // neighbor index (get convention)
        m1 = Psi[nn]; // get neighbor for phi - 1
        //........................................................................
        nn = ijk + 1; // neighbor index (get convention)
        m2 = Psi[nn]; // get neighbor for phi - 2
        //........................................................................
        nn = ijk - strideY; // neighbor index (get convention)
        m3 = Psi[nn];       // get neighbor for phi - 3
        //........................................................................
        nn = ijk + strideY; // neighbor index (get convention)
        m4 = Psi[nn];       // get neighbor for phi - 4
        //........................................................................
        nn = ijk - strideZ; // neighbor index (get convention)
        m5 = Psi[nn];       // get neighbor for phi - 5
        //........................................................................
        nn = ijk + strideZ; // neighbor index (get convention)
        m6 = Psi[nn];       // get neighbor for phi - 6
        //........................................................................
        nn = ijk - strideY - 1; // neighbor index (get convention)
        m7 = Psi[nn];           // get neighbor for phi - 7
        //........................................................................
        nn = ijk + strideY + 1; // neighbor index (get convention)
        m8 = Psi[nn];           // get neighbor for phi - 8
        //........................................................................
        nn = ijk + strideY - 1; // neighbor index (get convention)
        m9 = Psi[nn];           // get neighbor for phi - 9
        //........................................................................
        nn = ijk - strideY + 1; // neighbor index (get convention)
        m10 = Psi[nn];          // get neighbor for phi - 10
        //........................................................................
        nn = ijk - strideZ - 1; // neighbor index (get convention)
        m11 = Psi[nn];          // get neighbor for phi - 11
        //........................................................................
        nn = ijk + strideZ + 1; // neighbor index (get convention)
        m12 = Psi[nn];          // get neighbor for phi - 12
        //........................................................................
        nn = ijk + strideZ - 1; // neighbor index (get convention)
        m13 = Psi[nn];          // get neighbor for phi - 13
        //........................................................................
        nn = ijk - strideZ + 1; // neighbor index (get convention)
        m14 = Psi[nn];          // get neighbor for phi - 14
        //........................................................................
        nn = ijk - strideZ - strideY; // neighbor index (get convention)
        m15 = Psi[nn];                // get neighbor for phi - 15
        //........................................................................
        nn = ijk + strideZ + strideY; // neighbor index (get convention)
        m16 = Psi[nn];                // get neighbor for phi - 16
        //........................................................................
        nn = ijk + strideZ - strideY; // neighbor index (get convention)
        m17 = Psi[nn];                // get neighbor for phi - 17
        //........................................................................
        nn = ijk - strideZ + strideY; // neighbor index (get convention)
        m18 = Psi[nn];                // get neighbor for phi - 18

        psi_Laplacian =
            2.0 * 3.0 / 18.0 *
            (m1 + m2 + m3 + m4 + m5 + m6 - 6 * psi +
             0.5 * (m7 + m8 + m9 + m10 + m11 + m12 + m13 + m14 + m15 + m16 +
                    m17 + m18 - 12 * psi)); //Laplacian of electric potential
        residual_error = psi_Laplacian + rho_e / epsilon_LB;
        ResidualError[n] = residual_error;
    }
}
//extern "C" void ScaLBL_D3Q7_Poisson_ElectricField(int *neighborList, int *Map, signed char *ID, double *Psi, double *ElectricField, int SolidBC,
//        int strideY, int strideZ,int start, int finish, int Np){
//
//	int n,nn;
//    int ijk;
//    int id;
//	// distributions
//	double m1,m2,m3,m4,m5,m6,m7,m8,m9;
//	double m10,m11,m12,m13,m14,m15,m16,m17,m18;
//	double nx,ny,nz;
//
//	for (n=start; n<finish; n++){
//
//		// Get the 1D index based on regular data layout
//		ijk = Map[n];
//		//					COMPUTE THE COLOR GRADIENT
//		//........................................................................
//		//.................Read Phase Indicator Values............................
//		//........................................................................
//		nn = ijk-1;							// neighbor index (get convention)
//        id = ID[nn];
//		m1 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 1
//		//........................................................................
//		nn = ijk+1;							// neighbor index (get convention)
//        id = ID[nn];
//		m2 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 2
//		//........................................................................
//		nn = ijk-strideY;							// neighbor index (get convention)
//        id = ID[nn];
//		m3 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 3
//		//........................................................................
//		nn = ijk+strideY;							// neighbor index (get convention)
//        id = ID[nn];
//		m4 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 4
//		//........................................................................
//		nn = ijk-strideZ;						// neighbor index (get convention)
//        id = ID[nn];
//		m5 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 5
//		//........................................................................
//		nn = ijk+strideZ;						// neighbor index (get convention)
//        id = ID[nn];
//		m6 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 6
//		//........................................................................
//		nn = ijk-strideY-1;						// neighbor index (get convention)
//        id = ID[nn];
//		m7 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 7
//		//........................................................................
//		nn = ijk+strideY+1;						// neighbor index (get convention)
//        id = ID[nn];
//		m8 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 8
//		//........................................................................
//		nn = ijk+strideY-1;						// neighbor index (get convention)
//        id = ID[nn];
//		m9 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 9
//		//........................................................................
//		nn = ijk-strideY+1;						// neighbor index (get convention)
//        id = ID[nn];
//		m10 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 10
//		//........................................................................
//		nn = ijk-strideZ-1;						// neighbor index (get convention)
//        id = ID[nn];
//		m11 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 11
//		//........................................................................
//		nn = ijk+strideZ+1;						// neighbor index (get convention)
//        id = ID[nn];
//		m12 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 12
//		//........................................................................
//		nn = ijk+strideZ-1;						// neighbor index (get convention)
//        id = ID[nn];
//		m13 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 13
//		//........................................................................
//		nn = ijk-strideZ+1;						// neighbor index (get convention)
//        id = ID[nn];
//		m14 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 14
//		//........................................................................
//		nn = ijk-strideZ-strideY;					// neighbor index (get convention)
//        id = ID[nn];
//		m15 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 15
//		//........................................................................
//		nn = ijk+strideZ+strideY;					// neighbor index (get convention)
//        id = ID[nn];
//		m16 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 16
//		//........................................................................
//		nn = ijk+strideZ-strideY;					// neighbor index (get convention)
//        id = ID[nn];
//		m17 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 17
//		//........................................................................
//		nn = ijk-strideZ+strideY;					// neighbor index (get convention)
//        id = ID[nn];
//		m18 = SolidBC==1 ? Psi[nn] : Psi[nn]*(id>0)+Psi[ijk]*(id<=0);// get neighbor for phi - 18
//		//............Compute the Color Gradient...................................
//		//nx = 1.f/6.f*(m1-m2+0.5*(m7-m8+m9-m10+m11-m12+m13-m14));
//		//ny = 1.f/6.f*(m3-m4+0.5*(m7-m8-m9+m10+m15-m16+m17-m18));
//		//nz = 1.f/6.f*(m5-m6+0.5*(m11-m12-m13+m14+m15-m16-m17+m18));
//		nx = 1.f/6.f*(m1-m2);//but looks like it needs to multiply another factor of 3
//		ny = 1.f/6.f*(m3-m4);
//		nz = 1.f/6.f*(m5-m6);
//
//		ElectricField[n] = nx;
//		ElectricField[Np+n] = ny;
//		ElectricField[2*Np+n] = nz;
//	}
//}

extern "C" void ScaLBL_D3Q19_Poisson_getElectricField(double *dist, double *ElectricField, double tau, int Np){
	int n;
	double f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,
	f16, f17, f18;
	double Ex,Ey,Ez;
	double rlx=1.0/tau;

	for (n=0; n<Np; n++){
		//........................................................................
		// Registers to store the distributions
		//........................................................................
        f1 = dist[2 * Np + n];
        f2 = dist[1 * Np + n];
        f3 = dist[4 * Np + n];
        f4 = dist[3 * Np + n];
        f5 = dist[6 * Np + n];
        f6 = dist[5 * Np + n];
        f7 = dist[8 * Np + n];
        f8 = dist[7 * Np + n];
        f9 = dist[10 * Np + n];
        f10 = dist[9 * Np + n];
        f11 = dist[12 * Np + n];
        f12 = dist[11 * Np + n];
        f13 = dist[14 * Np + n];
        f14 = dist[13 * Np + n];
        f15 = dist[16 * Np + n];
        f16 = dist[15 * Np + n];
        f17 = dist[18 * Np + n];
        f18 = dist[17 * Np + n];
		//.................Compute the Electric Field...................................
		Ex = (f1 - f2 + f7 - f8 + f9 - f10 + f11 - f12 + f13 - f14)*rlx*3.0;//NOTE the unit of electric field here is V/lu
		Ey = (f3 - f4 + f7 - f8 - f9 + f10 + f15 - f16 + f17 - f18)*rlx*3.0;
		Ez = (f5 - f6 + f11 - f12 - f13 + f14 + f15 - f16 - f17 + f18)*rlx*3.0;
		//..................Write the Electric Field.....................................
		ElectricField[0*Np+n] = Ex;
		ElectricField[1*Np+n] = Ey;
		ElectricField[2*Np+n] = Ez;
		//........................................................................
	}
}

extern "C" void
ScaLBL_D3Q19_AAodd_Poisson_ElectricPotential(int *neighborList, int *Map,
                                            double *dist, double *Den_charge, double *Psi,
                                            double epsilon_LB, bool UseSlippingVelBC,
                                            int start, int finish, int Np) {
    int n;
    double psi;        //electric potential
    double rho_e;      //local charge density
    //double Gs;
    double f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,
        f16, f17, f18;
    int nr1, nr2, nr3, nr4, nr5, nr6, nr7, nr8, nr9, nr10, nr11, nr12, nr13,
        nr14, nr15, nr16, nr17, nr18;
    int idx;

    for (n = start; n < finish; n++) {
        rho_e = (UseSlippingVelBC==1) ? 0.0 : Den_charge[n] / epsilon_LB;

        // q=0
        f0 = dist[n];
        // q=1
        nr1 = neighborList[n]; // neighbor 2 ( > 10Np => odd part of dist)
        f1 = dist[nr1];        // reading the f1 data into register fq

        nr2 = neighborList[n + Np]; // neighbor 1 ( < 10Np => even part of dist)
        f2 = dist[nr2];             // reading the f2 data into register fq

        // q=3
        nr3 = neighborList[n + 2 * Np]; // neighbor 4
        f3 = dist[nr3];

        // q = 4
        nr4 = neighborList[n + 3 * Np]; // neighbor 3
        f4 = dist[nr4];

        // q=5
        nr5 = neighborList[n + 4 * Np];
        f5 = dist[nr5];

        // q = 6
        nr6 = neighborList[n + 5 * Np];
        f6 = dist[nr6];

        // q=7
        nr7 = neighborList[n + 6 * Np];
        f7 = dist[nr7];

        // q = 8
        nr8 = neighborList[n + 7 * Np];
        f8 = dist[nr8];

        // q=9
        nr9 = neighborList[n + 8 * Np];
        f9 = dist[nr9];

        // q = 10
        nr10 = neighborList[n + 9 * Np];
        f10 = dist[nr10];

        // q=11
        nr11 = neighborList[n + 10 * Np];
        f11 = dist[nr11];

        // q=12
        nr12 = neighborList[n + 11 * Np];
        f12 = dist[nr12];

        // q=13
        nr13 = neighborList[n + 12 * Np];
        f13 = dist[nr13];

        // q=14
        nr14 = neighborList[n + 13 * Np];
        f14 = dist[nr14];

        // q=15
        nr15 = neighborList[n + 14 * Np];
        f15 = dist[nr15];

        // q=16
        nr16 = neighborList[n + 15 * Np];
        f16 = dist[nr16];

        // q=17
        //fq = dist[18*Np+n];
        nr17 = neighborList[n + 16 * Np];
        f17 = dist[nr17];
        
        // q=18
        nr18 = neighborList[n + 17 * Np];
        f18 = dist[nr18];
        
        psi = f0 + f2 + f1 + f4 + f3 + f6 + f5 + f8 + f7 + f10 + f9 + f12 +
                f11 + f14 + f13 + f16 + f15 + f18 + f17;

        idx = Map[n];

        Psi[idx] = psi - 0.5*rho_e;
    }
}

extern "C" void ScaLBL_D3Q19_AAeven_Poisson_ElectricPotential(
    int *Map, double *dist, double *Den_charge, double *Psi, double epsilon_LB, bool UseSlippingVelBC, int start, int finish, int Np) {
    int n;
    double psi;        //electric potential
    double rho_e;      //local charge density
    double f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,
        f16, f17, f18;
    //double Gs;
    int idx;

    for (n = start; n < finish; n++) {
        rho_e = (UseSlippingVelBC==1) ? 0.0 : Den_charge[n] / epsilon_LB;

        //........................................................................
        // q=0
        f0 = dist[n];
        f1 = dist[2 * Np + n];
        f2 = dist[1 * Np + n];
        f3 = dist[4 * Np + n];
        f4 = dist[3 * Np + n];
        f5 = dist[6 * Np + n];
        f6 = dist[5 * Np + n];
        f7 = dist[8 * Np + n];
        f8 = dist[7 * Np + n];
        f9 = dist[10 * Np + n];
        f10 = dist[9 * Np + n];
        f11 = dist[12 * Np + n];
        f12 = dist[11 * Np + n];
        f13 = dist[14 * Np + n];
        f14 = dist[13 * Np + n];
        f15 = dist[16 * Np + n];
        f16 = dist[15 * Np + n];
        f17 = dist[18 * Np + n];
        f18 = dist[17 * Np + n];

        psi = f0 + f2 + f1 + f4 + f3 + f6 + f5 + f8 + f7 + f10 + f9 + f12 +
                f11 + f14 + f13 + f16 + f15 + f18 + f17;

        idx = Map[n];

        Psi[idx] = psi - 0.5*rho_e;
    }
}
extern "C" void ScaLBL_D3Q19_AAodd_Poisson(int *neighborList, int *Map,
                                          double *dist, double *Den_charge,
                                          double *Psi, double *ElectricField,
                                          double tau, double Vt, double Cp, double epsilon_LB, bool UseSlippingVelBC,
                                          int start, int finish, int Np) {
    int n;
    double psi;        //electric potential
    double Ex, Ey, Ez; //electric field
    double rho_e;      //local charge density
    double f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,
        f16, f17, f18;
    int nr1, nr2, nr3, nr4, nr5, nr6, nr7, nr8, nr9, nr10, nr11, nr12, nr13,
        nr14, nr15, nr16, nr17, nr18;
	double sum_q;
    double rlx = 1.0 / tau;
    int idx;

    double W0 = 0.5;
    double W1 = 1.0/24.0;
    double W2 = 1.0/48.0;
    
    for (n = start; n < finish; n++) {

        //Load data
        //When Helmholtz-Smoluchowski slipping velocity BC is used, the bulk fluid is considered as electroneutral
        //and thus the net space charge density is zero. 
        rho_e = (UseSlippingVelBC==1) ? 0.0 : Den_charge[n] / epsilon_LB;

        // q=0
        f0 = dist[n];
        // q=1
        nr1 = neighborList[n]; // neighbor 2 ( > 10Np => odd part of dist)
        f1 = dist[nr1];        // reading the f1 data into register fq

        nr2 = neighborList[n + Np]; // neighbor 1 ( < 10Np => even part of dist)
        f2 = dist[nr2];             // reading the f2 data into register fq

        // q=3
        nr3 = neighborList[n + 2 * Np]; // neighbor 4
        f3 = dist[nr3];

        // q = 4
        nr4 = neighborList[n + 3 * Np]; // neighbor 3
        f4 = dist[nr4];

        // q=5
        nr5 = neighborList[n + 4 * Np];
        f5 = dist[nr5];

        // q = 6
        nr6 = neighborList[n + 5 * Np];
        f6 = dist[nr6];
        
        // q=7
         nr7 = neighborList[n + 6 * Np];
         f7 = dist[nr7];

         // q = 8
         nr8 = neighborList[n + 7 * Np];
         f8 = dist[nr8];

         // q=9
         nr9 = neighborList[n + 8 * Np];
         f9 = dist[nr9];

         // q = 10
         nr10 = neighborList[n + 9 * Np];
         f10 = dist[nr10];

         // q=11
         nr11 = neighborList[n + 10 * Np];
         f11 = dist[nr11];

         // q=12
         nr12 = neighborList[n + 11 * Np];
         f12 = dist[nr12];

         // q=13
         nr13 = neighborList[n + 12 * Np];
         f13 = dist[nr13];

         // q=14
         nr14 = neighborList[n + 13 * Np];
         f14 = dist[nr14];

         // q=15
         nr15 = neighborList[n + 14 * Np];
         f15 = dist[nr15];

         // q=16
         nr16 = neighborList[n + 15 * Np];
         f16 = dist[nr16];

         // q=17
         //fq = dist[18*Np+n];
         nr17 = neighborList[n + 16 * Np];
         f17 = dist[nr17];

         // q=18
         nr18 = neighborList[n + 17 * Np];
         f18 = dist[nr18];

         sum_q = f1+f2+f3+f4+f5+f6+f7+f8+f9+f10+f11+f12+f13+f14+f15+f16+f17+f18;
         //error = 8.0*(sum_q - f0) + rho_e; 

         psi = 2.0*(f0*(1.0 - rlx) + rlx*(sum_q + 0.125*rho_e));
         
         idx = Map[n];
         Psi[idx] = psi;

         Ex = (f1 - f2 + 0.5*(f7 - f8 + f9 - f10 + f11 - f12 + f13 - f14))*4.0; //NOTE the unit of electric field here is V/lu
         Ey = (f3 - f4 + 0.5*(f7 - f8 - f9 + f10 + f15 - f16 + f17 - f18))*4.0;
         Ez = (f5 - f6 + 0.5*(f11 - f12 - f13 + f14 + f15 - f16 - f17 + f18))*4.0;
         ElectricField[n + 0 * Np] = Ex;
         ElectricField[n + 1 * Np] = Ey;
         ElectricField[n + 2 * Np] = Ez;

         // q = 0
         dist[n] = W0*psi; //f0 * (1.0 - rlx) -  (1.0-0.5*rlx)*W0*rho_e;

         // q = 1
         dist[nr2] = W1*psi; //f1 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 2
         dist[nr1] = W1*psi; //f2 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 3
         dist[nr4] = W1*psi; //f3 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 4
         dist[nr3] = W1*psi; //f4 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 5
         dist[nr6] = W1*psi; //f5 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 6
         dist[nr5] = W1*psi; //f6 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;
         //........................................................................

         // q = 7
         dist[nr8] = W2*psi; //f7 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 8
         dist[nr7] = W2*psi; //f8 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 9
         dist[nr10] = W2*psi; //f9 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 10
         dist[nr9] = W2*psi; //f10 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 11
         dist[nr12] = W2*psi; //f11 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 12
         dist[nr11] = W2*psi; //f12 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 13
         dist[nr14] = W2*psi; //f13 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q= 14
         dist[nr13] = W2*psi; //f14 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 15
         dist[nr16] = W2*psi; //f15 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 16
         dist[nr15] = W2*psi; //f16 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 17
         dist[nr18] = W2*psi; //f17 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 18
         dist[nr17] = W2*psi; //f18 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
    }
}

extern "C" void ScaLBL_D3Q19_AAeven_Poisson(int *Map, double *dist,
		double *Den_charge, double *Psi,
		double *ElectricField, double *Error, double tau,
		double Vt, double Cp,
		double epsilon_LB, bool UseSlippingVelBC,
		int start, int finish, int Np) {
	
	int n;
	double psi;        //electric potential
	double Ex, Ey, Ez; //electric field
	double rho_e;      //local charge density
	double f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,
	f16, f17, f18;
	double error,sum_q;
	double rlx = 1.0 / tau;
	int idx;
	double W0 = 0.5;
	double W1 = 1.0/24.0;
	double W2 = 1.0/48.0;

	for (n = start; n < finish; n++) {

		//Load data
		//When Helmholtz-Smoluchowski slipping velocity BC is used, the bulk fluid is considered as electroneutral
		//and thus the net space charge density is zero. 
		//rho_e = (UseSlippingVelBC==1) ? 0.0 : Den_charge[n] / epsilon_LB;
		rho_e = Den_charge[n] / epsilon_LB;

		f0 = dist[n];
		f1 = dist[2 * Np + n];
		f2 = dist[1 * Np + n];
		f3 = dist[4 * Np + n];
		f4 = dist[3 * Np + n];
		f5 = dist[6 * Np + n];
		f6 = dist[5 * Np + n];

		f7 = dist[8 * Np + n];
		f8 = dist[7 * Np + n];
		f9 = dist[10 * Np + n];
		f10 = dist[9 * Np + n];
		f11 = dist[12 * Np + n];
		f12 = dist[11 * Np + n];
		f13 = dist[14 * Np + n];
		f14 = dist[13 * Np + n];
		f15 = dist[16 * Np + n];
		f16 = dist[15 * Np + n];
		f17 = dist[18 * Np + n];
		f18 = dist[17 * Np + n];

		/* Ex = (f1 - f2) * rlx *
             4.0; //NOTE the unit of electric field here is V/lu
        Ey = (f3 - f4) * rlx *
             4.0; //factor 4.0 is D3Q7 lattice squared speed of sound
        Ez = (f5 - f6) * rlx * 4.0;
		 */
		Ex = (f1 - f2 + 0.5*(f7 - f8 + f9 - f10 + f11 - f12 + f13 - f14))*4.0; //NOTE the unit of electric field here is V/lu
		Ey = (f3 - f4 + 0.5*(f7 - f8 - f9 + f10 + f15 - f16 + f17 - f18))*4.0;
		Ez = (f5 - f6 + 0.5*(f11 - f12 - f13 + f14 + f15 - f16 - f17 + f18))*4.0;
		ElectricField[n + 0 * Np] = Ex;
		ElectricField[n + 1 * Np] = Ey;
		ElectricField[n + 2 * Np] = Ez;

		sum_q = f1+f2+f3+f4+f5+f6+f7+f8+f9+f10+f11+f12+f13+f14+f15+f16+f17+f18;
		error = 8.0*(sum_q - f0) + rho_e; 
		
		Error[n] = error;

		psi = 2.0*(f0*(1.0 - rlx) + rlx*(sum_q + 0.125*rho_e));
        
        idx = Map[n];
        Psi[idx] = psi;
        
		// q = 0
		dist[n] =  W0*psi;//

		// q = 1
		dist[1 * Np + n] =  W1*psi;//f1 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 2
		dist[2 * Np + n] =  W1*psi;//f2 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 3
		dist[3 * Np + n] =  W1*psi;//f3 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 4
		dist[4 * Np + n] =  W1*psi;//f4 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 5
		dist[5 * Np + n] =  W1*psi;//f5 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 6
		dist[6 * Np + n] =  W1*psi;//f6 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		dist[7 * Np + n] =  W2*psi;//f7 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[8 * Np + n] =  W2*psi;//f8* (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[9 * Np + n] =  W2*psi;//f9 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[10 * Np + n] = W2*psi;//f10 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[11 * Np + n] = W2*psi;//f11 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[12 * Np + n] = W2*psi;//f12 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[13 * Np + n] = W2*psi;//f13 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[14 * Np + n] = W2*psi;//f14 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[15 * Np + n] = W2*psi;//f15 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[16 * Np + n] = W2*psi;//f16 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[17 * Np + n] = W2*psi;//f17 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[18 * Np + n] = W2*psi;//f18 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

		//........................................................................
	}
}

/** **/
extern "C" void ScaLBL_D3Q19_AAodd_Poisson_Grotthus(int *neighborList, int *Map,
                                          double *dist, double *Den_charge,
                                          double *Psi, double *ElectricField,
                                          double tau, double Vt, double Cp,
                                          double epsilon_LB, bool UseSlippingVelBC,
                                          int start, int finish, int Np) {
    int n;
    double psi, psit;          		//electric potential
    double Ex, Ey, Ez; 				//electric field
    double rho_i, rho_p, rho_e;     //local charge density
    double f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,
        f16, f17, f18;
    int nr1, nr2, nr3, nr4, nr5, nr6, nr7, nr8, nr9, nr10, nr11, nr12, nr13,
        nr14, nr15, nr16, nr17, nr18;
	double sum_q;
    double rlx = 1.0 / tau;
    int idx;

    double W0 = 0.5;
    double W1 = 1.0/24.0;
    double W2 = 1.0/48.0;
    
    double F,G,Fprime;
    double factor = 1.0 / epsilon_LB;
    double inVt = 1.0 / Vt; 
    double expsum, expdiff, term, xv;
    
    /* exponential series coefficients */
    double a3 = 0.3333333333333333;
    double a4 = 0.25; //0.08333333333333333;
    double a5 = 0.2; // 0.01666666666666667;
    double a6 = 0.1666666666666667;//0.002777777777777778;
    double a7 = 0.1428571428571428; //0.0003968253968253968;
    double a8 = 0.125; //4.96031746031746e-05;
    double a9 = 0.1111111111111111; //5.511463844797179e-06;
    double a10 = 0.1; //5.511463844797178e-07;
    double a11 = 0.09090909090909091; //5.010421677088344e-08;
    double a12 = 0.08333333333333333; //4.17535139757362e-09;
    double a13 = 0.07692307692307693; 
    
    for (n = start; n < finish; n++) {

        //Load data
        //When Helmholtz-Smoluchowski slipping velocity BC is used, the bulk fluid is considered as electroneutral
        //and thus the net space charge density is zero. 
        rho_i = (UseSlippingVelBC==1) ? 0.0 :  Den_charge[n];

        // q=0
        f0 = dist[n];
        // q=1
        nr1 = neighborList[n]; // neighbor 2 ( > 10Np => odd part of dist)
        f1 = dist[nr1];        // reading the f1 data into register fq

        nr2 = neighborList[n + Np]; // neighbor 1 ( < 10Np => even part of dist)
        f2 = dist[nr2];             // reading the f2 data into register fq

        // q=3
        nr3 = neighborList[n + 2 * Np]; // neighbor 4
        f3 = dist[nr3];

        // q = 4
        nr4 = neighborList[n + 3 * Np]; // neighbor 3
        f4 = dist[nr4];

        // q=5
        nr5 = neighborList[n + 4 * Np];
        f5 = dist[nr5];

        // q = 6
        nr6 = neighborList[n + 5 * Np];
        f6 = dist[nr6];
        
        // q=7
         nr7 = neighborList[n + 6 * Np];
         f7 = dist[nr7];

         // q = 8
         nr8 = neighborList[n + 7 * Np];
         f8 = dist[nr8];

         // q=9
         nr9 = neighborList[n + 8 * Np];
         f9 = dist[nr9];

         // q = 10
         nr10 = neighborList[n + 9 * Np];
         f10 = dist[nr10];

         // q=11
         nr11 = neighborList[n + 10 * Np];
         f11 = dist[nr11];

         // q=12
         nr12 = neighborList[n + 11 * Np];
         f12 = dist[nr12];

         // q=13
         nr13 = neighborList[n + 12 * Np];
         f13 = dist[nr13];

         // q=14
         nr14 = neighborList[n + 13 * Np];
         f14 = dist[nr14];

         // q=15
         nr15 = neighborList[n + 14 * Np];
         f15 = dist[nr15];

         // q=16
         nr16 = neighborList[n + 15 * Np];
         f16 = dist[nr16];

         // q=17
         //fq = dist[18*Np+n];
         nr17 = neighborList[n + 16 * Np];
         f17 = dist[nr17];

         // q=18
         nr18 = neighborList[n + 17 * Np];
         f18 = dist[nr18];


 		Ex = (f1 - f2 + 0.5*(f7 - f8 + f9 - f10 + f11 - f12 + f13 - f14))*4.0; //NOTE the unit of electric field here is V/lu
 		Ey = (f3 - f4 + 0.5*(f7 - f8 - f9 + f10 + f15 - f16 + f17 - f18))*4.0;
 		Ez = (f5 - f6 + 0.5*(f11 - f12 - f13 + f14 + f15 - f16 - f17 + f18))*4.0;
 		ElectricField[n + 0 * Np] = Ex;
 		ElectricField[n + 1 * Np] = Ey;
 		ElectricField[n + 2 * Np] = Ez;

         sum_q = f1+f2+f3+f4+f5+f6+f7+f8+f9+f10+f11+f12+f13+f14+f15+f16+f17+f18;
         G = 8.0* sum_q  + rho_i*factor; 
         
         /* Use Poisson-Boltzmann for fast proton transport */
         psit = 4.0*f0;
         // rho_p = Cp * (exp(psi*inVt) - exp(-psi*inVt));
         // rho_e = rho_i + rho_p;
         
         /* use semi-implicit scheme */
         //Wt = W0 + Cp*inVt*factor*(1.0 + 0.16666666666666667*(psit*inVt)*(psit*inVt) + 0.00833333333333333*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt));


         for (int s=0; s<10; s++){
         	/* approximate the exponential with Taylor series */
         	expsum =  2.0; 
         	xv = (psit*inVt);
         	expdiff = 2.0*xv;
         	term = xv*xv;
         	expsum += term;
         	term *= a3*xv;
         	expdiff += term;
         	term *= a4*xv;
         	expsum += term;
         	term *= a5*xv;
         	expdiff += term;
         	term *= a6*xv;
         	expsum += term;
         	term *= a7*xv;
         	expdiff += term;
         	term *= a8*xv;    
         	expsum += term;
         	term *= a9*xv;
         	expdiff += term;
         	term *= a10*xv;
         	expsum += term;
         	term *= a11*xv;
         	expdiff += term;
         	term *= a12*xv;
         	expsum += term;
         	term *= a13*xv;
         	expdiff += term;
         	
         	/* Compare to analytical */
         	double truesum = exp(xv) + exp(-1.0*xv);
         	double truediff = exp(xv) - exp(-1.0*xv);
         	
         	expdiff = truediff;
         	expsum = truesum;
        
         	/* Newton iteration */
         	F = Cp*factor*expdiff - 8.0*W0*psit + G;
         	Fprime = Cp*factor*inVt*expsum - 8.0*W0;

         	psit -= (F / Fprime);
         	/* Newton iteration is successful if F=0 */

         }

         
         /* 1/ 5040 = 0.0001984126984126984     *(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt) */
         /* 1/ 362880 = 2.755731922398589e-06   *(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt) */
         /* 1/ 39916800 = 2.505210838544172e-08 *(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt) */

         /* compute new psi */
         psi = 2.0*f0*(1.0 - rlx) + rlx*psit;   //(1.0 / Wt)*(sum_q + 0.125*rho_i);   

         idx = Map[n];
         Psi[idx] = psi;

         // q = 0
         dist[n] = W0*psi; //f0 * (1.0 - rlx) -  (1.0-0.5*rlx)*W0*rho_e;

         // q = 1
         dist[nr2] = W1*psi; //f1 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 2
         dist[nr1] = W1*psi; //f2 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 3
         dist[nr4] = W1*psi; //f3 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 4
         dist[nr3] = W1*psi; //f4 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 5
         dist[nr6] = W1*psi; //f5 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

         // q = 6
         dist[nr5] = W1*psi; //f6 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;
         //........................................................................

         // q = 7
         dist[nr8] = W2*psi; //f7 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 8
         dist[nr7] = W2*psi; //f8 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 9
         dist[nr10] = W2*psi; //f9 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 10
         dist[nr9] = W2*psi; //f10 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 11
         dist[nr12] = W2*psi; //f11 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 12
         dist[nr11] = W2*psi; //f12 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 13
         dist[nr14] = W2*psi; //f13 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q= 14
         dist[nr13] = W2*psi; //f14 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 15
         dist[nr16] = W2*psi; //f15 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 16
         dist[nr15] = W2*psi; //f16 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 17
         dist[nr18] = W2*psi; //f17 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

         // q = 18
         dist[nr17] = W2*psi; //f18 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
    }
}

extern "C" void ScaLBL_D3Q19_AAeven_Poisson_Grotthus(int *Map, double *dist,
		double *Den_charge, double *Psi, double *ElectricField, double *Error,
		double tau, double Vt, double Cp,
		double epsilon_LB, bool UseSlippingVelBC,
		int start, int finish, int Np) {
	int n;
	double psi, psit;        //electric potential
	double Ex, Ey, Ez;       //electric field
	double rho_e, rho_i, rho_p;     //local charge density
	double f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15,
	f16, f17, f18;
	double error,sum_q;
	double rlx = 1.0 / tau;
	int idx;
	double W0 = 0.5;
	double W1 = 1.0/24.0;
	double W2 = 1.0/48.0;

    double F,G,Fprime;
    double factor = 1.0 / epsilon_LB;
    double inVt = 1.0 / Vt; 
    double expsum, expdiff, term, xv;
    
    /* exponential series coefficients */
    double a3 = 0.3333333333333333;
    double a4 = 0.25; //0.08333333333333333;
    double a5 = 0.2; // 0.01666666666666667;
    double a6 = 0.1666666666666667;//0.002777777777777778;
    double a7 = 0.1428571428571428; //0.0003968253968253968;
    double a8 = 0.125; //4.96031746031746e-05;
    double a9 = 0.1111111111111111; //5.511463844797179e-06;
    double a10 = 0.1; //5.511463844797178e-07;
    double a11 = 0.09090909090909091; //5.010421677088344e-08;
    double a12 = 0.08333333333333333; //4.17535139757362e-09;
    double a13 = 0.07692307692307693; 
    
	for (n = start; n < finish; n++) {

		//Load data
		//When Helmholtz-Smoluchowski slipping velocity BC is used, the bulk fluid is considered as electroneutral
		//and thus the net space charge density is zero. 
		rho_i = (UseSlippingVelBC==1) ? 0.0 : Den_charge[n];

		f0 = dist[n];
		f1 = dist[2 * Np + n];
		f2 = dist[1 * Np + n];
		f3 = dist[4 * Np + n];
		f4 = dist[3 * Np + n];
		f5 = dist[6 * Np + n];
		f6 = dist[5 * Np + n];

		f7 = dist[8 * Np + n];
		f8 = dist[7 * Np + n];
		f9 = dist[10 * Np + n];
		f10 = dist[9 * Np + n];
		f11 = dist[12 * Np + n];
		f12 = dist[11 * Np + n];
		f13 = dist[14 * Np + n];
		f14 = dist[13 * Np + n];
		f15 = dist[16 * Np + n];
		f16 = dist[15 * Np + n];
		f17 = dist[18 * Np + n];
		f18 = dist[17 * Np + n];

		/* Ex = (f1 - f2) * rlx *
             4.0; //NOTE the unit of electric field here is V/lu
        Ey = (f3 - f4) * rlx *
             4.0; //factor 4.0 is D3Q7 lattice squared speed of sound
        Ez = (f5 - f6) * rlx * 4.0;
		 */
		Ex = (f1 - f2 + 0.5*(f7 - f8 + f9 - f10 + f11 - f12 + f13 - f14))*4.0; //NOTE the unit of electric field here is V/lu
		Ey = (f3 - f4 + 0.5*(f7 - f8 - f9 + f10 + f15 - f16 + f17 - f18))*4.0;
		Ez = (f5 - f6 + 0.5*(f11 - f12 - f13 + f14 + f15 - f16 - f17 + f18))*4.0;
		ElectricField[n + 0 * Np] = Ex;
		ElectricField[n + 1 * Np] = Ey;
		ElectricField[n + 2 * Np] = Ez;

        sum_q = f1+f2+f3+f4+f5+f6+f7+f8+f9+f10+f11+f12+f13+f14+f15+f16+f17+f18;
        G = 8.0* sum_q  + rho_i*factor; 
        
        /* Use Poisson-Boltzmann for fast proton transport */
        psit = 4.0*f0;
        // rho_p = Cp * (exp(psi*inVt) - exp(-psi*inVt));
        // rho_e = rho_i + rho_p;
        
        /* use semi-implicit scheme */
        //Wt = W0 + Cp*inVt*factor*(1.0 + 0.16666666666666667*(psit*inVt)*(psit*inVt) + 0.00833333333333333*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt));

        for (int s=0; s<10; s++){
        	/* approximate the exponential with Taylor series */
        	expsum =  2.0; 
        	xv = (psit*inVt);
        	expdiff = 2.0*xv;
        	term = xv*xv;
        	expsum += term;
        	term *= a3*xv;
        	expdiff += term;
        	term *= a4*xv;
        	expsum += term;
        	term *= a5*xv;
        	expdiff += term;
        	term *= a6*xv;
        	expsum += term;
        	term *= a7*xv;
        	expdiff += term;
        	term *= a8*xv;    
        	expsum += term;
        	term *= a9*xv;
        	expdiff += term;
        	term *= a10*xv;
        	expsum += term;
        	term *= a11*xv;
        	expdiff += term;
        	term *= a12*xv;
        	expsum += term;
        	term *= a13*xv;
        	expdiff += term;
        	
        	/* Compare to analytical */
        	double truesum = exp(xv) + exp(-1.0*xv);
        	double truediff = exp(xv) - exp(-1.0*xv);
        	
        	expdiff = truediff;
        	expsum = truesum;
       
        	/*  iteration */
        	F = Cp*factor*expdiff - 8.0*W0*psit + G;
        	Fprime = Cp*factor*inVt*expsum - 8.0*W0;

        	psit -= (F / Fprime);
        	/* Newton iteration is successful if F=0 */

        }

    	
    	//if (fabs(expsum - truesum) > 1e-8)     printf("Error in sum (psi = %0.5g, Vt =%0.5g):  approx = %0.5g, true value =  %0.5g \n", psit, Vt, expsum, truesum);
    	//if (fabs(expdiff - truediff) > 1e-8)   printf("Error in diff:  approx = %0.5g, true value =  %0.5g \n", expdiff, truediff);
    	
        /* 1/ 5040 = 0.0001984126984126984     *(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt) */
        /* 1/ 362880 = 2.755731922398589e-06   *(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt) */
        /* 1/ 39916800 = 2.505210838544172e-08 *(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt)*(psit*inVt) */

        /* compute new psi */
        psi = 2.0*f0*(1.0 - rlx) + rlx*psit;   //(1.0 / Wt)*(sum_q + 0.125*rho_i);         
		//error = 8.0*(sum_q - f0) + rho_i*factor;
        error = Cp*factor*expdiff - 8.0*f0 + G;		
		Error[n] = error;
		
		
    	if (error > 1e-3){
   		 printf(" Newton's method error (site=%i) =  %0.5g \n",n,F);
    	} 
		
		
        idx = Map[n];
        Psi[idx] = psi;
        
		// q = 0
		dist[n] =  W0*psi;//

		// q = 1
		dist[1 * Np + n] =  W1*psi;//f1 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 2
		dist[2 * Np + n] =  W1*psi;//f2 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 3
		dist[3 * Np + n] =  W1*psi;//f3 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 4
		dist[4 * Np + n] =  W1*psi;//f4 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 5
		dist[5 * Np + n] =  W1*psi;//f5 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		// q = 6
		dist[6 * Np + n] =  W1*psi;//f6 * (1.0 - rlx) +W1* (rlx * psi) - (1.0-0.5*rlx)*0.05555555555555555*rho_e;

		dist[7 * Np + n] =  W2*psi;//f7 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[8 * Np + n] =  W2*psi;//f8* (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[9 * Np + n] =  W2*psi;//f9 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[10 * Np + n] = W2*psi;//f10 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[11 * Np + n] = W2*psi;//f11 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[12 * Np + n] = W2*psi;//f12 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[13 * Np + n] = W2*psi;//f13 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[14 * Np + n] = W2*psi;//f14 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[15 * Np + n] = W2*psi;//f15 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[16 * Np + n] = W2*psi;//f16 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[17 * Np + n] = W2*psi;//f17 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;
		dist[18 * Np + n] = W2*psi;//f18 * (1.0 - rlx) +W2* (rlx * psi) - (1.0-0.5*rlx)*0.02777777777777778*rho_e;

		//........................................................................
	}
}

extern "C" void ScaLBL_D3Q19_AAeven_Poisson_Potential_BC_z(int *list,  double *dist, double Vin, int count, int Np) {
	//double W0 = 0.5;
	double W1 = 1.0/24.0;
	double W2 = 1.0/48.0;
    int n;//nread, nr5;
    
	double psi = Vin;

    for (int idx = 0; idx < count; idx++) {
        n = list[idx];
		
        dist[6 * Np + n]  = W1*psi;
        dist[12 * Np + n] = W2*psi;
        dist[13 * Np + n] = W2*psi;
        dist[16 * Np + n] = W2*psi;
        dist[17 * Np + n] = W2*psi;
    }
}

extern "C" void ScaLBL_D3Q19_AAeven_Poisson_Potential_BC_Z(int *list,
                                                          double *dist,
                                                          double Vout,
                                                          int count, int Np) {
	//double W0 = 0.5;	
	double W1 = 1.0/24.0;
	double W2 = 1.0/48.0;
	
	double psi = Vout;

	for (int idx = 0; idx < count; idx++) {
		
        int n = list[idx];
        
        dist[5 * Np + n]  = W1*psi;
        dist[11 * Np + n] = W2*psi;
        dist[14 * Np + n] = W2*psi;
        dist[15 * Np + n] = W2*psi;
        dist[18 * Np + n] = W2*psi;
	}
}

extern "C" void ScaLBL_D3Q19_AAodd_Poisson_Potential_BC_z(int *d_neighborList,
                                                         int *list,
                                                         double *dist,
                                                         double Vin, int count,
                                                         int Np) {
	double W1 = 1.0/24.0;
	double W2 = 1.0/48.0;

    int nr5, nr11, nr14, nr15, nr18;

    double psi = Vin;
    
    for (int idx = 0; idx < count; idx++) {
    	int n = list[idx];
    	nr5  = d_neighborList[n + 4 * Np];
    	nr11 = d_neighborList[n + 10 * Np];
    	nr14 = d_neighborList[n + 13 * Np];
    	nr15 = d_neighborList[n + 14 * Np];
    	nr18 = d_neighborList[n + 17 * Np];

        dist[nr5]  = W1*psi;
        dist[nr11] = W2*psi;
        dist[nr14] = W2*psi;
        dist[nr15] = W2*psi;
        dist[nr18] = W2*psi;
    }
}

extern "C" void ScaLBL_D3Q19_AAodd_Poisson_Potential_BC_Z(int *d_neighborList, int *list, double *dist, double Vout, int count, int Np)  {
	
	double W1 = 1.0/24.0;
	double W2 = 1.0/48.0;
    int nr6, nr12, nr13, nr16, nr17;

    double psi = Vout;
    
	for (int idx = 0; idx < count; idx++) {
    	int n = list[idx];

    	nr6 = d_neighborList[n + 5 * Np];
    	nr12 = d_neighborList[n + 11 * Np];
    	nr13 = d_neighborList[n + 12 * Np];
    	nr16 = d_neighborList[n + 15 * Np];
    	nr17 = d_neighborList[n + 16 * Np];
    	
        dist[nr6]  = W1*psi;
        dist[nr12] = W2*psi;
        dist[nr13] = W2*psi;
        dist[nr16] = W2*psi;
        dist[nr17] = W2*psi;
	}
}

extern "C" void ScaLBL_D3Q19_Poisson_Init(int *Map, double *dist, double *Psi,
		int start, int finish, int Np) {
	int n;
	int ijk;
	double W0 = 0.5;
	double W1 = 1.0/24.0;
	double W2 = 1.0/48.0;
	for (n = start; n < finish; n++) {
		ijk = Map[n];
		dist[0 * Np + n] = W0 * Psi[ijk];//3333333333333333* Psi[ijk];
		dist[1 * Np + n] = W1 * Psi[ijk];
		dist[2 * Np + n] = W1 * Psi[ijk];
		dist[3 * Np + n] = W1 * Psi[ijk];
		dist[4 * Np + n] = W1 * Psi[ijk];
		dist[5 * Np + n] = W1 * Psi[ijk];
		dist[6 * Np + n] = W1 * Psi[ijk];
		dist[7 * Np + n] = W2* Psi[ijk];
		dist[8 * Np + n] = W2* Psi[ijk];
		dist[9 * Np + n] = W2* Psi[ijk];
		dist[10 * Np + n] = W2* Psi[ijk];
		dist[11 * Np + n] = W2* Psi[ijk];
		dist[12 * Np + n] = W2* Psi[ijk];
		dist[13 * Np + n] = W2* Psi[ijk];
		dist[14 * Np + n] = W2* Psi[ijk];
		dist[15 * Np + n] = W2* Psi[ijk];
		dist[16 * Np + n] = W2* Psi[ijk];
		dist[17 * Np + n] = W2* Psi[ijk];
		dist[18 * Np + n] = W2* Psi[ijk];
	}
}
