/* ****************************************************************************
 * 
 * 				KeyPair.h
 * 
 * Author: Nedim Srndic
 * Release date: 17th of June 2008
 * 
 * A class representing a public/private RSA keypair. 
 * 
 * A keypair consists of a public key and a matching private key. 
 * 
 * ****************************************************************************
 */

#ifndef NET_BASE_RSA_KEYPAIR_H_
#define NET_BASE_RSA_KEYPAIR_H_

//#pragma GCC visibility push(hidden)
#include "net/base/net_export.h"
#include "Key.h"
#include <iostream>

namespace net {

class NET_EXPORT KeyPair
{
	private:
		const Key privateKey;
		const Key publicKey;
	public:
		KeyPair(Key privateKey, Key publicKey): 
			privateKey(privateKey), publicKey(publicKey)
		{}
		const Key &GetPrivateKey() const
		{
			return privateKey;
		}
		const Key &GetPublicKey() const
		{
			return publicKey;
		}
		friend std::ostream &operator <<(std::ostream &cout, const KeyPair &k);
};

} // namespace net
//#pragma GCC visibility pop

#endif /*NET_BASE_RSA_KEYPAIR_H_*/
