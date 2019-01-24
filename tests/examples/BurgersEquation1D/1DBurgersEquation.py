#!/usr/bin/env python
# -*-coding:utf-8 -*

#===============================================================================================================================
# Name        : Résolution VF de l'équation du Burgers 1D \partial_t u + 1/2 \partial_x u^2 = 0 
# Author      : Michaël Ndjinga
# Copyright   : CEA Saclay 2019
# Description : Utilisation du schéma de Godunov explicite sur un maillage 1D régulier
#               Conditions aux limites de Neumann
#		        Création et sauvegarde du champ résultant et des figures
#               Génération d'une video sauvegardée dans un fichier .mp4
#================================================================================================================================


from math import sin, cos, pi
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.animation as manimation
import sys

def Flux_Godunov(u_l, u_r):
    if (u_l==u_r):
        flux = 0.5*u_l*u_l
    elif (u_l<0 and 0<u_r):
        fluxs = 0.;
    elif (u_l<u_r):
        flux = min(0.5*u_l*u_l,0.5*u_r*u_r);
    elif (u_l>u_r):
        flux = max(0.5*u_l*u_l,0.5*u_r*u_r);
    return flux

def Burgers1D():
    ##################### Simulation parameters
    a = 0.0 # space domain :  a <= x <= b
    b = 1.
    nx=100
    dx = (b - a) / nx #space step

    tmax = 0.9 # runs the simulation for 0 <= t <= tMax
    ntmax=100
    cfl=0.95

    x=[a+0.5*dx + i*dx for i in range(nx)]   # array of cell center (1D mesh)
    
    ########################## Initial data
    xa=0.1
    xb=0.5
    u_initial = [ (xi<xa)+(xa<=xi)*(xi<=xb)*(np.cos(np.pi*(xa-xi)/(xa-xb))+1.0)*0.5  for xi in x];# to be used with a=0, b=1
    u_godunov        = [ (xi<xa)+(xa<=xi)*(xi<=xb)*(np.cos(np.pi*(xa-xi)/(xa-xb))+1.0)*0.5  for xi in x];# to be used with a=0, b=1
    Unp1        = [ (xi<xa)+(xa<=xi)*(xi<=xb)*(np.cos(np.pi*(xa-xi)/(xa-xb))+1.0)*0.5  for xi in x];# to be used with a=0, b=1

    max_initial=max(u_initial)
    min_initial=min(u_initial)

    time = 0.
    it = 0
    output_freq = 10

    # Video settings
    FFMpegWriter = manimation.writers['ffmpeg']
    metadata = dict(title="Finite volumes schemes for Burgers equation", artist = "CEA Saclay", comment="Non linear wave propagation")
    writer=FFMpegWriter(fps=output_freq, metadata=metadata, codec='h264')
    with writer.saving(plt.figure(), "1DBurgersEquation_FV"+".mp4", ntmax):
        ########################### Postprocessing initialisation
        # Picture frame
        plt.legend()
        plt.xlabel('x')
        plt.ylabel('u')
        plt.xlim(a,b)
        plt.ylim( min_initial - 0.1*(max_initial-min_initial), max_initial +  0.1*(max_initial-min_initial) )
        plt.title('Finite volume schemes for Burgers equation')
        line1, = plt.plot(x, u_godunov, label='Godunov scheme') #new picture for video # Returns a tuple of line objects, thus the comma
    
        print("Starting time loop")
        print("-- Iter: " + str(it) + ", Time: " + str(time) )
        np.savetxt("BurgersEquation_FVGodunov_ResultField_0"+".txt", u_godunov, delimiter="\n")
        writer.grab_frame()
        plt.savefig("BurgersEquation_FVGodunov_ResultField_0"+".png")

        ############################# Time loop
        while (it < ntmax and time <= tmax):
            upw = max(np.abs(u_godunov))
            dt = cfl * dx / upw
            # Loop on all cells
            for i in xrange(0,nx):
                if (i==0):
                    flux_iminus = 0.5*u_godunov[0]*u_godunov[0]#Flux at the left Neumann boundary
                if (i==nx-1):
                    flux_iplus  = 0.5*u_godunov[nx-1]*u_godunov[nx-1]#Flux at the right Neumann boundary
                else:
                    flux_iplus = Flux_Godunov(u_godunov[i],u_godunov[i+1])
                pass
                Unp1[i] = u_godunov[i] - dt/dx*(flux_iplus-flux_iminus);
                flux_iminus = flux_iplus;
            pass
            u_godunov = Unp1
    
            time += dt
            it += 1

            # Postprocessing
            line1.set_ydata(u_godunov)
            writer.grab_frame()
            if (it % output_freq == 0):
                print("-- Iter: " + str(it) + ", Time: " + str(time) + ", dt: " + str(dt))
                np.savetxt( "BurgersEquation_FVGodunov_ResultField_"+str(it)+".txt", u_godunov, delimiter="\n")
                plt.savefig("BurgersEquation_FVGodunov_ResultField_"+str(it)+".png")
                #plt.show()
    
    print("Simulation of Burgers equation with finite volume schemes done.")
    
    return

if __name__ == """__main__""":
    Burgers1D()
