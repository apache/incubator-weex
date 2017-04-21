/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXScannerHistoryVC.h"
#import "DemoDefine.h"
#import "WXDemoViewController.h"
#import "UIViewController+WXDemoNaviBar.h"
#import <CoreText/CoreText.h>

@interface WXScannerHistoryVC ()
@property(nonatomic)NSMutableArray * scanner_history;
@property(nonatomic)UIImageView * qrCodeImageView;
@end

@implementation WXScannerHistoryVC

- (instancetype)init {
    if (self = [super init]) {
        _scanner_history = [[[NSUserDefaults standardUserDefaults] objectForKey:WX_SCANNER_HISTORY] mutableCopy];
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setupNaviBar];
    UITapGestureRecognizer * tapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleClick:)];
    [self.tableView addGestureRecognizer:tapGestureRecognizer];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reloadHistory) name:NSUserDefaultsDidChangeNotification object:nil];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    return 88.0f;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [_scanner_history count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView registerClass:[UITableViewCell class] forCellReuseIdentifier:@"WX_SCANNER_HISTORY_CELL"];
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"WX_SCANNER_HISTORY_CELL" forIndexPath:indexPath];
    if (!cell) {
        cell = [UITableViewCell new];
    }
    UILongPressGestureRecognizer * longPressGesture = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongPress:)];
    longPressGesture.minimumPressDuration = 1.0;
    [cell addGestureRecognizer:longPressGesture];
    NSMutableAttributedString * attributeString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithFormat:@"%ld %@",(long)indexPath.row+1,[_scanner_history objectAtIndex:indexPath.row]]];
    [attributeString addAttribute:(NSString*)NSForegroundColorAttributeName value:(id)[UIColor redColor].CGColor range:NSMakeRange(0, 1)];
    NSDictionary<NSString*, id> *attributes = @{
                                               NSForegroundColorAttributeName:[UIColor redColor],
                                               NSFontAttributeName: [UIFont boldSystemFontOfSize:25.0],
                                               };
    [attributeString addAttributes:attributes range:NSMakeRange(0, 2)];
    cell.textLabel.attributedText = attributeString;
    cell.textLabel.numberOfLines = 0;
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSString *urlStr = _scanner_history[indexPath.row];
    if (urlStr) {
        WXDemoViewController * demoVC = [WXDemoViewController new];
        demoVC.url = [NSURL URLWithString:urlStr];
        [self.navigationController pushViewController:demoVC animated:YES];
    }
}

- (void)reloadHistory {
    if ([_scanner_history count]) {
        [_qrCodeImageView removeFromSuperview];
        [_scanner_history removeAllObjects];
        [self.tableView reloadData];
    }
}

- (void)handleClick:(UITapGestureRecognizer*)gestureRecognizer {
    CGPoint clickPoint = [gestureRecognizer locationInView:self.tableView];
    if (!CGRectContainsPoint(_qrCodeImageView.frame, clickPoint)) {
        [_qrCodeImageView removeFromSuperview];
    }
    
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)gestureRecognizer {
    if (gestureRecognizer.state != UIGestureRecognizerStateBegan) {
        return ;
    }
    CGPoint pressPoint = [gestureRecognizer locationInView:self.tableView];
    NSIndexPath *indexPath = [self.tableView indexPathForRowAtPoint:pressPoint];
    if (indexPath == nil) {
        return;
    }
    
    NSString * url = _scanner_history[indexPath.row];
    UIImage * qrCodeImage = [self getQRCodeWithData:url size:CGSizeMake(300, 300)];
    _qrCodeImageView.contentMode = UIViewContentModeScaleAspectFill;
    if (![_qrCodeImageView superview]) {
        _qrCodeImageView = [[UIImageView alloc] initWithImage:qrCodeImage];
        [_qrCodeImageView setCenter:self.view.center];
        [self.view addSubview:_qrCodeImageView];
    }else {
        _qrCodeImageView.image = qrCodeImage;
    }
}

- (UIImage*)getQRCodeWithData:(NSString*)url size:(CGSize)size {
    CIFilter *filter = [CIFilter filterWithName:@"CIQRCodeGenerator"];
    [filter setDefaults];
    NSString *info = url;
    NSData *infoData = [info dataUsingEncoding:NSUTF8StringEncoding];
    [filter setValue:infoData forKeyPath:@"inputMessage"];
    
    CIImage *outputImage = [filter outputImage];
    CGRect extent = CGRectIntegral(outputImage.extent);
    CGFloat scale = MIN(size.width/CGRectGetWidth(extent), size.width/CGRectGetHeight(extent));
    
    size_t width = CGRectGetWidth(extent) * scale;
    size_t height = CGRectGetHeight(extent) * scale;
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceGray();
    CGContextRef bitmapRef = CGBitmapContextCreate(nil, width, height, 8, 0, cs, (CGBitmapInfo)kCGImageAlphaNone);
    CIContext *context = [CIContext contextWithOptions:nil];
    CGImageRef bitmapImage = [context createCGImage:outputImage fromRect:extent];
    CGContextSetInterpolationQuality(bitmapRef, kCGInterpolationNone);
    CGContextScaleCTM(bitmapRef, scale, scale);
    CGContextDrawImage(bitmapRef, extent, bitmapImage);
    
    CGImageRef scaledImage = CGBitmapContextCreateImage(bitmapRef);
    CGContextRelease(bitmapRef);
    CGImageRelease(bitmapImage);
    return [UIImage imageWithCGImage:scaledImage];
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
