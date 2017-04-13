/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#import "WXModuleProtocol.h"
#import "WXType.h"

@protocol WXImageOperationProtocol <NSObject>

- (void)cancel;

@end

@protocol WXImgLoaderProtocol <WXModuleProtocol>

/**
 * @abstract Creates a image download handler with a given URL
 *
 * @param url The URL of the image to download
 *
 * @param imageFrame  The frame of the image you want to set
 *
 * @param options : The options to be used for this download
 *
 * @param completedBlock : A block called once the download is completed.
 *                 image : the image which has been download to local.
 *                 error : the error which has happened in download.
 *              finished : a Boolean value indicating whether download action has finished.
 */
- (id<WXImageOperationProtocol>)downloadImageWithURL:(NSString *)url imageFrame:(CGRect)imageFrame userInfo:(NSDictionary *)options completed:(void(^)(UIImage *image,  NSError *error, BOOL finished))completedBlock;

@end
