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

#import "WXVideoComponent.h"
#import "WXHandlerFactory.h"
#import "WXURLRewriteProtocol.h"
#import "WXSDKEngine.h"
#import "WXImgLoaderProtocol.h"

#import <AVFoundation/AVPlayer.h>
#import <AVKit/AVPlayerViewController.h>
#import <MediaPlayer/MPMoviePlayerViewController.h>
#import <MediaPlayer/MPMoviePlayerController.h>
#import <AVFoundation/AVPlayerItem.h>

@interface WXPlayer : NSObject

@end

@implementation WXPlayer

@end

@interface WXVideoView()

@property (nonatomic, strong) UIViewController *playerViewController;
@property (nonatomic, strong) AVPlayerItem *playerItem;
@property (nonatomic, strong) WXSDKInstance *weexSDKInstance;
@property (nonatomic, strong) UIImageView *posterImageView;
@property (nonatomic, strong) id<WXImageOperationProtocol> imageOperation;
@property (nonatomic, assign) BOOL playerDidPlayed;

@end

@implementation WXVideoView

- (id)init
{
    if (self = [super init]) {
        if ([self greater8SysVer]) {
            _playerViewController = [AVPlayerViewController new];
            
        } else {
            _playerViewController = [[MPMoviePlayerViewController alloc] initWithContentURL:nil];
            MPMoviePlayerViewController *MPVC = (MPMoviePlayerViewController*)_playerViewController;
            MPVC.moviePlayer.controlStyle = MPMovieControlStyleEmbedded;
            MPVC.moviePlayer.shouldAutoplay = NO;
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(playFinish)
                                                         name:MPMoviePlayerPlaybackDidFinishNotification
                                                       object:MPVC.moviePlayer];
            [[NSNotificationCenter defaultCenter] addObserverForName:MPMoviePlayerPlaybackStateDidChangeNotification object:MPVC.moviePlayer queue:nil usingBlock:^(NSNotification *notification)
             {
                 if (MPVC.moviePlayer.playbackState == MPMoviePlaybackStatePlaying) {
                     if (_playbackStateChanged)
                         _playbackStateChanged(WXPlaybackStatePlaying);
                 }
                 if (MPVC.moviePlayer.playbackState == MPMoviePlaybackStateStopped) {
                     //stopped
                 } if (MPVC.moviePlayer.playbackState == MPMoviePlaybackStatePaused) {
                     //paused
                     if (_playbackStateChanged) {
                         _playbackStateChanged(WXPlaybackStatePaused);
                     }
                 } if (MPVC.moviePlayer.playbackState == MPMoviePlaybackStateInterrupted) {
                     //interrupted
                 } if (MPVC.moviePlayer.playbackState == MPMoviePlaybackStateSeekingForward) {
                     //seeking forward
                 } if (MPVC.moviePlayer.playbackState == MPMoviePlaybackStateSeekingBackward) {
                     //seeking backward
                 }
             }];
        }
        
        [self addSubview:_playerViewController.view];
    }
    return self;
}

- (void)dealloc
{
    _weexSDKInstance = nil;
    if ([self greater8SysVer]) {
        AVPlayerViewController *AVVC = (AVPlayerViewController*)_playerViewController;
        [AVVC.player removeObserver:self forKeyPath:@"rate"];
        [_playerItem removeObserver:self forKeyPath:@"status"];
        
        [[NSNotificationCenter defaultCenter] removeObserver:self name:AVPlayerItemDidPlayToEndTimeNotification object: _playerItem];
    }
    else {
        MPMoviePlayerViewController *MPVC = (MPMoviePlayerViewController*)_playerViewController;
        [[NSNotificationCenter defaultCenter] removeObserver:self name:MPMoviePlayerPlaybackStateDidChangeNotification object:MPVC.moviePlayer];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:MPMoviePlayerPlaybackDidFinishNotification object:MPVC.moviePlayer];
    }
}

- (BOOL)greater8SysVer
{
    //return NO;
    NSString *reqSysVer = @"8.0";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    return [currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending;
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSString *,id> *)change
                       context:(void *)context
{
    if ([keyPath isEqualToString:@"rate"]) {
        float rate = [change[NSKeyValueChangeNewKey] floatValue];
        if (rate == 0.0) {
            if (_playbackStateChanged)
                _playbackStateChanged(WXPlaybackStatePaused);
        } else if (rate == 1.0) {
            if (_playbackStateChanged)
                _playbackStateChanged(WXPlaybackStatePlaying);
        } else if (rate == -1.0) {
            // Reverse playback
        }
    } else if ([keyPath isEqualToString:@"status"]) {
        NSInteger status = [change[NSKeyValueChangeNewKey] integerValue];
        if (status == AVPlayerStatusFailed) {
            if (_playbackStateChanged)
                _playbackStateChanged(WXPlaybackStateFailed);
        }
    }
}

- (void)setFrame:(CGRect)frame
{
    [super setFrame:frame];
    
    CGRect videoFrame = frame;
    videoFrame.origin.x = 0;
    videoFrame.origin.y = 0;
    [_playerViewController.view setFrame:videoFrame];
}

- (void)setURL:(NSURL *)URL
{
    NSString *newURL = [URL.absoluteString copy];
    WX_REWRITE_URL(URL.absoluteString, WXResourceTypeVideo, self.weexSDKInstance)
    
    if (!newURL) {
        return;
    }
    NSURL *videoNewURL = [NSURL URLWithString:newURL];
    if ([self greater8SysVer]) {
        
        AVPlayerViewController *AVVC = (AVPlayerViewController*)_playerViewController;
        if (AVVC.player && _playerItem) {
            [_playerItem removeObserver:self forKeyPath:@"status"];
            [AVVC.player removeObserver:self forKeyPath:@"rate"];
            [[NSNotificationCenter defaultCenter] removeObserver:self name:AVPlayerItemDidPlayToEndTimeNotification object: _playerItem];
        }
        _playerItem = [[AVPlayerItem alloc] initWithURL:videoNewURL];
        AVPlayer *player = [AVPlayer playerWithPlayerItem: _playerItem];
        AVVC.player = player;
        
        [player addObserver:self
                 forKeyPath:@"rate"
                    options:NSKeyValueObservingOptionNew
                    context:NULL];
        
        [_playerItem addObserver:self
                     forKeyPath:@"status"
                        options:NSKeyValueObservingOptionNew
                        context:NULL];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playFinish) name:AVPlayerItemDidPlayToEndTimeNotification object: _playerItem];
    }
    else {
        MPMoviePlayerViewController *MPVC = (MPMoviePlayerViewController*)_playerViewController;
        [MPVC moviePlayer].contentURL = videoNewURL;
    }
}

- (void)setPosterURL:(NSURL *)posterURL
{
    if (!posterURL) {
        return;
    }
    
    [self cancelImage];
    __weak typeof(self) weakSelf = self;
    weakSelf.imageOperation = [[self imageLoader] downloadImageWithURL:posterURL.absoluteString imageFrame:self.posterImageView.frame
                                                              userInfo:@{@"instanceId":self.weexSDKInstance.instanceId}
                                                             completed:^(UIImage *image, NSError *error, BOOL finished)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            __strong typeof(self) strongSelf = weakSelf;
            if (!error) {
                strongSelf.posterImageView.image = image;
                strongSelf.posterImageView.hidden = strongSelf.playerDidPlayed;
            }
        });
    }];
}

- (void)setControlShow:(BOOL)showControl
{
    if ([self greater8SysVer]) {
        AVPlayerViewController *AVVC = (AVPlayerViewController*)_playerViewController;
        AVVC.showsPlaybackControls = showControl;
    }
    else
    {
        MPMoviePlayerViewController *MPVC = (MPMoviePlayerViewController*)_playerViewController;
        MPVC.moviePlayer.controlStyle = showControl ? MPMovieControlStyleEmbedded : MPMovieControlStyleNone;
    }
}

- (void)playFinish
{
    if (_playbackStateChanged)
        _playbackStateChanged(WXPlaybackStatePlayFinish);
    if ([self greater8SysVer]) {
        AVPlayerViewController *AVVC = (AVPlayerViewController*)_playerViewController;
        [[AVVC player] seekToTime:CMTimeMultiply([AVVC player].currentTime, 0)];
    } else {
        MPMoviePlayerViewController *MPVC = (MPMoviePlayerViewController*)_playerViewController;
        [[MPVC moviePlayer] stop];
    }
}

- (void)play
{
    _posterImageView.hidden = YES;
    if ([self greater8SysVer]) {
        AVPlayerViewController *AVVC = (AVPlayerViewController*)_playerViewController;

        [[AVVC player] play];
    } else {
        MPMoviePlayerViewController *MPVC = (MPMoviePlayerViewController*)_playerViewController;
        [[MPVC moviePlayer] play];
    }
}

- (void)pause
{
    if ([self greater8SysVer]) {
        AVPlayerViewController *AVVC = (AVPlayerViewController*)_playerViewController;
        [[AVVC player] pause];
    } else {
        MPMoviePlayerViewController *MPVC = (MPMoviePlayerViewController*)_playerViewController;
        [[MPVC moviePlayer] pause];
    }
}

- (void)posterTapHandler {
    if (self.posterClickHandle) {
        self.posterClickHandle();
    }
}

- (UIImageView *)posterImageView {
    if (!_posterImageView) {
        _posterImageView = [[UIImageView alloc] initWithFrame:self.bounds];
        _posterImageView.userInteractionEnabled = YES;
        [_posterImageView addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(posterTapHandler)]];
        _posterImageView.hidden = YES;
        [self addSubview:_posterImageView];
        [self bringSubviewToFront:_posterImageView];
    }
    return _posterImageView;
}

- (id<WXImgLoaderProtocol>)imageLoader
{
    static id<WXImgLoaderProtocol> imageLoader;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        imageLoader = [WXHandlerFactory handlerForProtocol:@protocol(WXImgLoaderProtocol)];
    });
    return imageLoader;
}

- (void)cancelImage
{
    [_imageOperation cancel];
    _imageOperation = nil;
}

@end

@interface WXVideoComponent()

@property (nonatomic, weak) WXVideoView *videoView;
@property (nonatomic, strong) NSURL *videoURL;
@property (nonatomic, strong) NSURL *posterURL;
@property (nonatomic) BOOL autoPlay;
@property (nonatomic) BOOL playStatus;
@property (nonatomic) BOOL showControl;

@end

@implementation WXVideoComponent

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance {
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    if (self) {
        if (attributes[@"src"]) {
            _videoURL = [NSURL URLWithString: attributes[@"src"]];
        }
        if (attributes[@"autoPlay"]) {
            _autoPlay = [attributes[@"autoPlay"] boolValue];
        }
        if ([attributes[@"playStatus"] compare:@"play" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
            _playStatus = true;
        }
        if ([attributes[@"playStatus"] compare:@"pause" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
            _playStatus = false;
        }
        if (attributes[@"poster"]) {
            _posterURL = [NSURL URLWithString: attributes[@"poster"]];
        }
        if (attributes[@"controls"]) {
            _showControl = ![attributes[@"controls"] isEqualToString:@"nocontrols"];
        }
    }
    return self;
}

- (UIView *)loadView
{
    WXVideoView* videoView = [[WXVideoView alloc] init];
    videoView.weexSDKInstance = self.weexInstance;
    return videoView;
}

- (void)viewDidLoad
{
    _videoView = (WXVideoView *)self.view;
    _videoView.layer.mask = [self drawBorderRadiusMaskLayer:_videoView.bounds];
    [_videoView setURL:_videoURL];
    [_videoView setPosterURL:_posterURL];
    [_videoView setControlShow:_showControl];
    
    __weak __typeof__(self) weakSelf = self;
    _videoView.posterClickHandle = ^{
        [weakSelf.videoView play];
    };
    _videoView.playbackStateChanged = ^(WXPlaybackState state) {
        NSString *eventType = nil;
        switch (state) {
            case WXPlaybackStatePlaying:
                eventType = @"start";
                break;
            case WXPlaybackStatePaused:
                eventType = @"pause";
                break;
            case WXPlaybackStatePlayFinish:
                eventType = @"finish";
                break;
            case WXPlaybackStateFailed:
                eventType = @"fail";
                break;
                
            default:
                NSCAssert(NO, @"");
                break;
        }
        [weakSelf fireEvent:eventType params:nil];
    };
    if (_playStatus) {
        [_videoView play];
    } else {
        [_videoView pause];
    }
    if (_autoPlay) {
        [_videoView play];
    }
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"src"]) {
        _videoURL = [NSURL URLWithString: attributes[@"src"]];
        [_videoView setURL:_videoURL];
    }
    if (attributes[@"autoPlay"]) {
        _autoPlay = [attributes[@"autoPlay"] boolValue];
        [_videoView play];
    }
    if ([attributes[@"playStatus"] compare:@"play" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
        _playStatus = true;
        [_videoView play];
    }
    if ([attributes[@"playStatus"] compare:@"pause" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
        _playStatus = false;
        [_videoView pause];
    }
    if (attributes[@"poster"]) {
        _posterURL = [NSURL URLWithString: attributes[@"poster"]];
        [_videoView setPosterURL:_posterURL];
    }
    if (attributes[@"controls"]) {
        _showControl = ![attributes[@"controls"] isEqualToString:@"nocontrols"];
        [_videoView setControlShow:_showControl];
    }
}

@end
