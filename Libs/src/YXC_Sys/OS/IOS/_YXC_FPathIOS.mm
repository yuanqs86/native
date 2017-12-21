/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */

#define __MODULE__ "YK.FilePath.IOS"

#include <AVFoundation/AVFoundation.h>
#include <YXC_Sys/YXC_FilePath.h>
#include <YXC_Sys/YXC_ErrMacros.hpp>

namespace YXC_Inner
{
    YXC_Status _YXC_FPathFindSpecDir_IOS(echar* path_base, YXC_FPathSpecDir dir)
    {
        NSString* nsDir;
        switch (dir)
        {
            case YXC_FPATH_SDIR_DOCUMENTS:
                nsDir = [NSSearchPathForDirectoriesInDomains(NSSharedPublicDirectory, NSUserDomainMask, YES) objectAtIndex:0];
                [nsDir getCString:path_base maxLength:YXC_MAX_PATH encoding:NSUTF8StringEncoding];
                
                YXC_FPathAppend(path_base, YC("Documents"));
                break;
            case YXC_FPATH_SDIR_APPDATA:
                nsDir = [NSSearchPathForDirectoriesInDomains(NSSharedPublicDirectory, NSUserDomainMask, YES) objectAtIndex:0];
                [nsDir getCString:path_base maxLength:YXC_MAX_PATH encoding:NSUTF8StringEncoding];
                YXC_FPathAppend(path_base, YC("AppData"));
                break;
            case YXC_FPATH_SDIR_BINARIES:
                YXC_FPathCopy(path_base, YC("/bin"));
                break;
            default:
                _YXC_REPORT_NEW_RET(YXC_ERC_INVALID_PATH, YC("Invalid special directory to find"));
                break;
        }
        return YXC_ERC_SUCCESS;
    }
    
}