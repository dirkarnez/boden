#include <bdn/init.h>
#include <bdn/mainThread.h>

#import <UiKit/UiKit.h>


@interface SimpleCallableWrapper : NSObject
{
    bdn::P<bdn::ISimpleCallable> pCallable;
}

@property bdn::P<bdn::ISimpleCallable> pCallable;

- (void)invoke;

@end

@implementation SimpleCallableWrapper

@synthesize pCallable;

-(void)invoke
{
    pCallable->call();
}

@end


namespace bdn
{
	

void CallFromMainThreadBase_::dispatch()
{
    SimpleCallableWrapper* wrapper = [[SimpleCallableWrapper alloc] init];
    wrapper.pCallable = this;
    
    [wrapper performSelectorOnMainThread:@selector(invoke)
                                 withObject:nil
                              waitUntilDone:NO];
    
    // release our reference. The selector will keep its own reference while
    // it exists.
    [wrapper release];
}


}

