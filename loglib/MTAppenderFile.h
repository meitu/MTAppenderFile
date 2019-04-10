//
//  MTAppenderFile.h
//  MTAppenderFile
//
//  Created by EuanC on 16/12/2017.
//  Copyright © 2017 meitu.com. All rights reserved.
//

#import <Foundation/Foundation.h>


#define kMTAppenderFileVersion 0.2.4


@interface MTAppenderFile : NSObject

- (instancetype)initWithFileDir:(NSString *)fileDir name:(NSString *)name;

- (void)open;
- (void)close;

- (void)appendText:(NSString *)text;

- (void)appendUTF8Text:(const char *)utf8Text;

/**
 @param format format content to write, length limit 3000.
 */
- (void)appendFormat:(const char *)format, ...;

@end
