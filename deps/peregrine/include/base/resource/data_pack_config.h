

#ifndef BASE_RESOURCE_DATA_PACK_CONFIG_H_
#define BASE_RESOURCE_DATA_PACK_CONFIG_H_

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "base/strings/string_piece.h"
#include "base/files/file_path.h"
#include "base/base_export.h"


namespace base {

class IDataPackConfig
{
public:
	virtual void Init( base::FilePath const& jsonCfg ) = 0;
	virtual std::wstring GetString( uint16 key ) = 0;
};
	
BASE_EXPORT IDataPackConfig* GetDataPackConfig( std::string const& cfgUuid = "" );

}  // namespace base

#endif  // BASE_RESOURCE_DATA_PACK_CONFIG_H_
