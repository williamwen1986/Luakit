//
//  ViewController.m
//  WeatherTest
//
//  Created by williamwen on 2018/5/31.
//  Copyright © 2018年 williamwen. All rights reserved.
//
#import "WeatherManager.h"
#import "ViewController.h"

@interface ViewController ()<UITableViewDataSource ,UITableViewDelegate>
@property (nonatomic, strong) NSArray *weathers;
@property (nonatomic, strong) UITableView *tableView;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    self.tableView =  [[UITableView alloc] initWithFrame:self.view.bounds style:UITableViewStylePlain];
    self.tableView.dataSource = self;
    self.tableView.delegate = self;
    [self.view addSubview:self.tableView];
    
    self.weathers = [WeatherManager getWeather];
    __weak ViewController* weakSelf = self;
    [WeatherManager loadWeather:^(NSArray *w){
        if (w.count) {
            weakSelf.weathers = w;
            [weakSelf.tableView reloadData];
        }
    }];
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
    return self.weathers.count;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell * cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:@"test"];
    cell.textLabel.text = [NSString stringWithFormat:@"%@ 日出日落：%@",self.weathers[indexPath.row][@"city"],self.weathers[indexPath.row][@"sun_info"]];
    cell.detailTextLabel.text = [NSString stringWithFormat:@"最高温度：%@ 最低温度：%@ %@ %@",self.weathers[indexPath.row][@"high"],self.weathers[indexPath.row][@"low"],self.weathers[indexPath.row][@"wind_direction"],self.weathers[indexPath.row][@"wind"]];
    return cell;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
