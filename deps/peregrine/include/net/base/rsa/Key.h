/* ****************************************************************************
 * 
 * 				Key.h
 * 
 * Author: Nedim Srndic
 * Release date: 16th of June 2008
 * 
 * A class representing a public or private RSA key. 
 * 
 * A public or private RSA key consists of a modulus and an exponent. In this 
 * implementation an object of type BigInt is used to store those values. 
 * 
 * ****************************************************************************
 */

#ifndef NET_BASE_RSA_KEY_H_
#define NET_BASE_RSA_KEY_H_

//#pragma GCC visibility push(hidden)
#include "net/base/net_export.h"
#include "BigInt.h"
#include <iostream>

namespace net {

class NET_EXPORT Key
{
	private:
		BigInt modulus;
		BigInt exponent;
	public:
		Key(const BigInt &modulus, const BigInt &exponent) :
			modulus(modulus), exponent(exponent)
		{}
		const BigInt &GetModulus() const
		{
			return modulus;
		}
		const BigInt &GetExponent() const
		{
			return exponent;
		}
		friend std::ostream &operator<<(std::ostream &cout, const Key &key);
};

}// namespace net 

//#pragma GCC visibility pop

#endif /*NET_BASE_RSA_KEY_H_*/
