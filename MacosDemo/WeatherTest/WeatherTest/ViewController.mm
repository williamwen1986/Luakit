//
//  ViewController.m
//  WeatherTest
//
//  Created by larpoux on 16/09/2019.
//  Copyright © 2019 larpoux. All rights reserved.
//

#import "ViewController.h"
#import "WeatherManager.h"



@implementation ViewController

NSArray *weathers;

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Do any additional setup after loading the view.
}


- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}



- (IBAction)TestButton:(id)sender
{
    weathers = [WeatherManager getWeather];
    [    WeatherManager loadWeather:^(NSArray *w)
        {
            if (w.count)
            {
                weathers = w;
            }
        }
    ];
}


@end
