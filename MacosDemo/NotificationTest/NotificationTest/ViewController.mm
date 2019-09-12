//
//  ViewController.m
//  NotificationTest
//
//  Created by larpoux on 11/09/2019.
//  Copyright © 2019 larpoux. All rights reserved.
//

#import "ViewController.h"
#include "base/memory/scoped_ptr.h"
#import "NotificationProxyObserver.h"

@interface ViewController()<NotificationProxyObserverDelegate>
{
    scoped_ptr<NotificationProxyObserver> _notification_observer;
}

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    _notification_observer.reset(new NotificationProxyObserver(self));
    _notification_observer->AddObserver(3);
}


- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (IBAction)Test1Btn:(id)sender
{
    post_notification(3, @"Button: #1");
}

- (IBAction)Test2Btn:(id)sender
{
    post_notification(3, @"Button: #2");
}

- (IBAction)Test3Btn:(id)sender
{
    post_notification(3, @"Button: #3");
}

- (IBAction)Test4Btn:(id)sender
{
    post_notification(3, @"Button: #4");
}

- (IBAction)Test5Btn:(id)sender
{
    post_notification(3, @"Button: #5");
}

- (IBAction)Test6Btn:(id)sender
{
    post_notification(3, @"Button: #6");
}

- (IBAction)Test7Btn:(id)sender
{
    post_notification(3, @"Button: #7");
}

- (IBAction)Test8Btn:(id)sender
{
    post_notification(3, @"Button: #8");
}

- (IBAction)Test9Btn:(id)sender
{
    post_notification(3, @"Button: #9");
}

- (IBAction)Test10Btn:(id)sender
{
    post_notification(3, @"Button: #10");
}


- (void)onNotification:(int)type data:(id)data
{
    NSLog(@"object-c onNotification type = %d data = %@", type , data);
}


@end
