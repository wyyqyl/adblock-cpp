/*!
 * \file Adblock.h
 *
 * \author yorath
 * \date October 11, 2013
 *
 * \details 
 */

#pragma once


#include "IAdblock.h"


namespace NS_ADBLOCK {

  /**
   * Implementation of interface IAdblock
   */
  class Adblock: public IAdblock {
  public:
    Adblock();
    ~Adblock();
  };

}


