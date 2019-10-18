//
//  ViewController.m
//  NotificationTest
//
//  Created by williamwen on 2018/6/5.
//  Copyright © 2018年 williamwen. All rights reserved.
//

#import "ViewController.h"
#include "base/memory/scoped_ptr.h"
#import "NotificationProxyObserver.h"
@interface ViewController()<UITableViewDataSource ,UITableViewDelegate,NotificationProxyObserverDelegate>{
    scoped_ptr<NotificationProxyObserver> _notification_observer;
}
@property (nonatomic, strong) UITableView *tableView;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    _notification_observer.reset(new NotificationProxyObserver(self));
    _notification_observer->AddObserver(3);
    // Do any additional setup after loading the view, typically from a nib.
    self.tableView =  [[UITableView alloc] initWithFrame:self.view.bounds style:UITableViewStylePlain];
    self.tableView.dataSource = self;
    self.tableView.delegate = self;
    [self.view addSubview:self.tableView];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 60;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 10;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell * cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:@"test"];
    cell.textLabel.text = [NSString stringWithFormat:@"test %@",@(indexPath.row)];
    return cell;
}

-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    post_notification(3, @{@"row":@(indexPath.row)});
}

- (void)onNotification:(int)type data:(id)data
{
    NSLog(@"object-c onNotification type = %d data = %@", type , data);
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
