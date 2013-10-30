/*****************************************************************/
/*    NAME: Michael Benjamin                                     */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: AOF_Contact.h                                        */
/*    DATE: May 17th, 2013 (Generalizing over existing classes)  */
/*                                                               */
/* This is unreleased BETA code. No permission is granted or     */
/* implied to use, copy, modify, and distribute this software    */
/* except by the author(s).                                      */
/*****************************************************************/
 
#ifndef AOF_CONTACT_HEADER
#define AOF_CONTACT_HEADER

#include <vector>
#include "AOF.h"
#include "CPAEngine.h"

class AOF_Contact: public AOF {
public:
  AOF_Contact(IvPDomain);
  ~AOF_Contact() {};

public: // virtual functions   
  virtual double evalPoint(const std::vector<double>&) const {return(0);};
  virtual bool   setParam(const std::string&, double);
  virtual bool   setParam(const std::string&, const std::string&) {return(false);};
  virtual bool   initialize();
  
  void setOwnshipParams(double osx, double osy);
  void setContactParams(double cnx, double cny, double cnh, double cnv);
  
protected:
  double m_tol;    // Ownship Time on Leg
  double m_osx;    // Ownship X position (meters)
  double m_osy;    // Ownship Y position (meters)
  double m_cnx;    // Contact X position (meters)
  double m_cny;    // Contact Y position (meters)
  double m_cnh;    // Contact heading
  double m_cnv;    // Contact speed

  double m_pwt_inner_distance;
  double m_pwt_outer_distance;
  double m_collision_distance;
  double m_all_clear_distance;

  bool   m_tol_set;
  bool   m_osy_set;
  bool   m_osx_set;
  bool   m_cnx_set;
  bool   m_cny_set;
  bool   m_cnh_set;
  bool   m_cnv_set;
  bool   m_collision_distance_set;
  bool   m_all_clear_distance_set;
  bool   m_pwt_inner_distance_set;
  bool   m_pwt_outer_distance_set;

  CPAEngine m_cpa_engine;
};

#endif





