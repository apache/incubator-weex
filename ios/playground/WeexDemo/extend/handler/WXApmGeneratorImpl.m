#import "WXApmGeneratorImpl.h"
#import "WXApmImpl.h"

@implementation WXApmGeneratorImpl

- (id<WXApmProtocol>)gengratorApmInstance:(NSString *) type
{
    id<WXApmProtocol> instance = [[WXApmImpl alloc] init];
    return instance;
}

@end
