/*  This file is part of MED.
 *
 *  COPYRIGHT (C) 1999 - 2015  EDF R&D, CEA/DEN
 *  MED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  MED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with MED.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <med.h>
#include <med_config.h>
#include <med_outils.h>

#include <string.h>
#include <stdlib.h>

med_err
MEDpolyedreInfo(med_idt fid, char *maa,med_connectivite type_conn,med_int *nf,
		med_int *consize)
{
  med_idt  maaid=0, entid=0, geoid=0, dataset=0;
  med_err  _ret = -1;
  char chemin[MED_TAILLE_MAA+MED_TAILLE_NOM+1];
  char nom_ent[MED_TAILLE_NOM_ENTITE+1];
  char nom_geo[MED_TAILLE_NOM_ENTITE+1];
  char nom_dataset[MED_TAILLE_NOM_ENTITE+1];
  med_int res = 0;
  med_entite_maillage type_ent;
  med_geometrie_element type_geo;

  /*
   * On inhibe le gestionnaire d'erreur HDF 5
   */
  _MEDmodeErreurVerrouiller();
if (MEDcheckVersion(fid) < 0) goto ERROR;


  /*
   * Si le maillage n'existe pas => erreur
   */
  strcpy(chemin,MED_MAA);
  strcat(chemin,maa);
  if ((maaid = _MEDdatagroupOuvrir(fid,chemin)) < 0)
    goto ERROR;

  /*
   * Acces au groupe HDF des entites (MED_MAILLE)
   */
  type_ent = MED_MAILLE;
  if ( _MEDnomEntite(nom_ent,type_ent) < 0)
    goto ERROR;
  if ((entid = _MEDdatagroupOuvrir(maaid,nom_ent)) < 0)
    goto ERROR;

  /*
   * Acces au groupe HDF du type geometrique MED_POLYEDRE
   */
  type_geo = MED_POLYEDRE;
  if ( _MEDnomGeometrie30(nom_geo,type_geo) < 0)
    goto ERROR;
  if ((geoid = _MEDdatagroupOuvrir(entid,nom_geo)) < 0)
    goto ERROR;
  
   /*
    * Ouverture du dataset HDF correspondant au mode de connectivite 
    * S'il n'existe pas => erreur
    * Sinon lecture de l'attribut HDF "TAI" qui designe la taille
    * du tableau des connectivites et de l'attribut "TTI" qui
    * designe la taille du tableau "indexf"
    */ 
   switch(type_conn)
     {
     case MED_NOD :
       strcpy(nom_dataset,MED_NOM_NOD);
       break;

     case MED_DESC :
       strcpy(nom_dataset,MED_NOM_DES);
       break;
       
     default :
       goto ERROR;
     }

   if ((dataset = _MEDdatasetOuvrir(geoid,nom_dataset)) < 0)
     goto ERROR;
   if ( _MEDattrEntierLire(dataset,MED_NOM_TAI,consize) < 0)
     goto ERROR;
   if ( _MEDdatasetFermer(dataset) < 0)
     goto ERROR;

   if ((dataset = _MEDdatasetOuvrir(geoid,nom_dataset)) < 0)
     goto ERROR;
   if ( _MEDattrEntierLire(dataset,MED_NOM_TTI,nf) < 0)
     goto ERROR;
   if (_MEDdatasetFermer(dataset) < 0)
     goto ERROR;

   /*
    * On ferme tout
    */
   _ret=0;

 ERROR:

   if (geoid > 0) if (_MEDdatagroupFermer(geoid) < 0) _ret= -1;
   if (entid > 0) if (_MEDdatagroupFermer(entid) < 0) _ret= -1;
   if (maaid > 0) if (_MEDdatagroupFermer(maaid) < 0)  _ret= -1;

  return _ret;
}
