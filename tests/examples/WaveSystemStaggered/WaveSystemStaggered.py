#!/usr/bin/env python
# -*-coding:utf-8 -*

from math import sin, cos, pi
import cdmath
import PV_routines
import VTK_routines
import sys

rho0=1000#reference density
c0=1500#reference sound speed
p0=rho0*c0*c0#reference pressure
precision=1e-5

def initial_conditions_wave_system(my_mesh):
    dim     = my_mesh.getMeshDimension()
    nbCells = my_mesh.getNumberOfCells()

    rayon = 0.15
    xcentre = 0.5
    ycentre = 0.5
    zcentre = 0.5

    pressure_field = cdmath.Field("Pressure",            cdmath.CELLS, my_mesh, 1)
    velocity_field = cdmath.Field("Velocity",            cdmath.CELLS, my_mesh, 3)

    for i in range(nbCells):
        x = my_mesh.getCell(i).x()
        y = my_mesh.getCell(i).y()

        velocity_field[i,0] = 0
        velocity_field[i,1] = 0
        velocity_field[i,2] = 0

        valX = (x - xcentre) * (x - xcentre)
        valY = (y - ycentre) * (y - ycentre)
        if(dim==2):
            val =  sqrt(valX + valY)
        if(dim==3):
            z = my_mesh.getCell(i).z()
            valZ = (z - zcentre) * (z - zcentre)
            val =  sqrt(valX + valY + valZ)
            
        if val < rayon:
            pressure_field[i] = p0
            pass
        else:
            pressure_field[i] = p0/2
            pass
        pass
        
    return pressure_field, velocity_field

def computeDivergenceMatrix(my_mesh,nbVoisinsMax,dt):
    nbCells = my_mesh.getNumberOfCells()
    dim=my_mesh.getMeshDimension()
    nbComp=dim+1
    normal=cdmath.Vector(dim)

    implMat=cdmath.SparseMatrixPetsc(nbCells*nbComp,nbCells*nbComp,(nbVoisinsMax+1)*nbComp)

    idMoinsJacCL=cdmath.Matrix(nbComp)

    v0=cdmath.Vector(dim)
    for i in range(dim) :
    for j in range(nbCells):#On parcourt les cellules
                
    return implMat

def WaveSystemStaggered(ntmax, tmax, cfl, my_mesh, output_freq, meshName, resolution):
    dim=my_mesh.getMeshDimension()
    nbCells = my_mesh.getNumberOfCells()
    
    dt = 0.
    time = 0.
    it=0;
    isStationary=False;
    
    nbVoisinsMax=my_mesh.getMaxNbNeighbours(CELLS)
    iterGMRESMax=50
    
    #iteration vectors
    Un=cdmath.Vector(nbCells*(dim+1))
    dUn=cdmath.Vector(nbCells*(dim+1))
    
    # Initial conditions #
    print("Construction of the initial condition …")
    pressure_field, velocity_field = initial_conditions_wave_system(my_mesh)

    for k in range(nbCells):
        Un[k*(dim+1)+0] =      pressure_field[k]
        Un[k*(dim+1)+1] = rho0*velocity_field[k,0]
        Un[k*(dim+1)+2] = rho0*velocity_field[k,1]
        if(dim==3):
            Un[k*(dim+1)+3] = rho0*velocity_field[k,2]
            
    #sauvegarde de la donnée initiale
    pressure_field.setTime(time,it);
    pressure_field.writeVTK("WaveSystem"+str(dim)+"DPStag"+meshName+"_pressure");
    velocity_field.setTime(time,it);
    velocity_field.writeVTK("WaveSystem"+str(dim)+"DPStag"+meshName+"_velocity");
    #Postprocessing : save 2D picture
    PV_routines.Save_PV_data_to_picture_file("WaveSystem"+str(dim)+"DPStag"+meshName+"_pressure"+'_0.vtu',"Pressure",'CELLS',"WaveSystem"+str(dim)+"DPStag"+meshName+"_pressure_initial")
    PV_routines.Save_PV_data_to_picture_file("WaveSystem"+str(dim)+"DPStag"+meshName+"_velocity"+'_0.vtu',"Velocity",'CELLS',"WaveSystem"+str(dim)+"DPStag"+meshName+"_velocity_initial")
    
    dx_min=my_mesh.minRatioVolSurf()

    dt = cfl * dx_min / c0

    divMat=computeDivergenceMatrix(my_mesh,nbVoisinsMax,dt)

    # Add the identity matrix on the diagonal
    for j in range(nbCells*(dim+1)):
        divMat.addValue(j,j,1.)
    LS=cdmath.LinearSolver(divMat,Un,iterGMRESMax, precision, "GMRES","LU")

    print("Starting computation of the linear wave system with an pseudo staggered scheme …")
    
    # Starting time loop
    while (it<ntmax and time <= tmax and not isStationary):
        dUn=Un.deepCopy()
        LS.setSndMember(Un)
        Un=LS.solve();
        cvgceLS=LS.getStatus();
        iterGMRES=LS.getNumberOfIter();
        if(not cvgceLS):
            print "Linear system did not converge ", iterGMRES, " GMRES iterations"
            raise ValueError("Pas de convergence du système linéaire");
        dUn-=Un
        
        maxVector=dUn.maxVector(dim+1)
        isStationary= maxVector[0]/p0<precision and maxVector[1]/rho0<precision and maxVector[2]/rho0<precision;
        if(dim==3):
            isStationary=isStationary and maxVector[3]/rho0<precision
        time=time+dt;
        it=it+1;
    
        #Sauvegardes
        if(it%output_freq==0 or it>=ntmax or isStationary or time >=tmax):
            print"-- Iter: " + str(it) + ", Time: " + str(time) + ", dt: " + str(dt)
            print "Variation temporelle relative : pressure ", maxVector[0]/p0 ,", velocity x", maxVector[1]/rho0 ,", velocity y", maxVector[2]/rho0
            print "Linear system converged in ", iterGMRES, " GMRES iterations"

            for k in range(nbCells):
                pressure_field[k]=Un[k*(dim+1)+0]
                velocity_field[k,0]=Un[k*(dim+1)+1]/rho0
                if(dim>1):
                    velocity_field[k,1]=Un[k*(dim+1)+2]/rho0
                    if(dim>2):
                        velocity_field[k,2]=Un[k*(dim+1)+3]/rho0
                
            pressure_field.setTime(time,it);
            pressure_field.writeVTK("WaveSystem"+str(dim)+"DPStag"+meshName+"_pressure",False);
            velocity_field.setTime(time,it);
            velocity_field.writeVTK("WaveSystem"+str(dim)+"DPStag"+meshName+"_velocity",False);

    print"-- Iter: " + str(it) + ", Time: " + str(time) + ", dt: " + str(dt)
    print "Variation temporelle relative : pressure ", maxVector[0]/p0 ,", velocity x", maxVector[1]/rho0 ,", velocity y", maxVector[2]/rho0
    print

    if(it>=ntmax):
        print "Nombre de pas de temps maximum ntmax= ", ntmax, " atteint"
    elif(isStationary):
        print "Régime stationnaire atteint au pas de temps ", it, ", t= ", time
        print "------------------------------------------------------------------------------------"

        pressure_field.setTime(time,0);
        pressure_field.writeVTK("WaveSystem"+str(dim)+"DPStag"+meshName+"_pressure_Stat");
        velocity_field.setTime(time,0);
        velocity_field.writeVTK("WaveSystem"+str(dim)+"DPStag"+meshName+"_velocity_Stat");

        #Postprocessing : save 2D picture
        PV_routines.Save_PV_data_to_picture_file("WaveSystem"+str(dim)+"DPStag"+meshName+"_pressure_Stat"+'_0.vtu',"Pressure",'CELLS',"WaveSystem"+str(dim)+"DPStag"+meshName+"_pressure_Stat")
        PV_routines.Save_PV_data_to_picture_file("WaveSystem"+str(dim)+"DPStag"+meshName+"_velocity_Stat"+'_0.vtu',"Velocity",'CELLS',"WaveSystem"+str(dim)+"DPStag"+meshName+"_velocity_Stat")
        
    else:
        print "Temps maximum Tmax= ", tmax, " atteint"


def solve(my_mesh,meshName,resolution):
    print "Resolution of the Wave system in dimension ", my_mesh.getSpaceDimension()
    print "Numerical method : immplicit pseudo staggered"
    print "Initial data : constant pressure, divergence free velocity"
    print "Wall boundary conditions"
    print "Mesh name : ",meshName , my_mesh.getNumberOfCells(), " cells"
    
    # Problem data
    tmax = 1000.
    ntmax = 100
    cfl = 1./my_mesh.getSpaceDimension()
    output_freq = 100

    WaveSystemStaggered(ntmax, tmax, cfl, my_mesh, output_freq, meshName, resolution)

def solve_file( filename,meshName, resolution):
    my_mesh = cdmath.Mesh(filename+".med")

    return solve(my_mesh, meshName+str(my_mesh.getNumberOfCells()),resolution)
    

if __name__ == """__main__""":
	if len(sys.argv) >1 :
		my_mesh = cdmath.Mesh(sys.argv[1])
		solve(my_mesh,my_mesh.getName(),100)
	else :
		raise ValueError("WaveSystemPStag.py expects a mesh file name")