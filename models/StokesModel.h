/*
 * Multi-relaxation time LBM Model
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <fstream>

#include "common/ScaLBL.h"
#include "common/Communication.h"
#include "common/MPI_Helpers.h"
#include "analysis/Minkowski.h"
#include "ProfilerApp.h"

class ScaLBL_StokesModel{
public:
	ScaLBL_StokesModel(int RANK, int NP, MPI_Comm COMM);
	~ScaLBL_StokesModel();	
	
	// functions in they should be run
	void ReadParams(string filename,int num_iter);
	void ReadParams(string filename);
	void ReadParams(std::shared_ptr<Database> db0);
	void SetDomain();
	void ReadInput();
	void Create();
	void Initialize();
	void Run();
	void Run_Lite(double *ChargeDensity, double *ElectricField);
	void VelocityField();
    void getVelocity(int timestep);
    void getVelocity_debug(int timestep);
    double CalVelocityConvergence(double& flow_rate_previous,double *ChargeDensity, double *ElectricField);
	
	bool Restart,pBC;
	int timestep,timestepMax;
	int BoundaryCondition;
	double tau,mu;
    double rho0;
	double Fx,Fy,Fz,flux;
	double din,dout;
	double tolerance;
    double nu_phys;
    double rho_phys;
    double time_conv;
    double h;//image resolution
    double den_scale;//scale factor for density
	
	int Nx,Ny,Nz,N,Np;
	int rank,nprocx,nprocy,nprocz,nprocs;
	double Lx,Ly,Lz;

	std::shared_ptr<Domain> Dm;   // this domain is for analysis
	std::shared_ptr<Domain> Mask; // this domain is for lbm
	std::shared_ptr<ScaLBL_Communicator> ScaLBL_Comm;
    // input database
    std::shared_ptr<Database> db;
    std::shared_ptr<Database> domain_db;
    std::shared_ptr<Database> stokes_db;

    IntArray Map;
    DoubleArray Distance;
    int *NeighborList;
    double *fq;
    double *Velocity;
    double *Pressure;
    
    //Minkowski Morphology;
    DoubleArray Velocity_x;
    DoubleArray Velocity_y;
    DoubleArray Velocity_z;
private:
	MPI_Comm comm;
	
	// filenames
    char LocalRankString[8];
    char LocalRankFilename[40];
    char LocalRestartFile[40];
    char OutputFilename[200];
   
    //int rank,nprocs;
    void LoadParams(std::shared_ptr<Database> db0);    	
    void Velocity_LB_to_Phys(DoubleArray &Vel_reg);
    vector<double> computeElectricForceAvg(double *ChargeDensity, double *ElectricField);
};
