--Script to populate the Newspaper's Info schema for F23 CSE 180 Lab1

-- SubscriptionKinds(subscriptionMode, subscriptionInterval, rate, stillOffered)
COPY SubscriptionKinds FROM stdin USING DELIMITERS '|';
D|1 years|70.00|TRUE
P|1 years|90.00|FALSE
B|1 years|160.00|TRUE
D|4 weeks|16.00|TRUE
P|4 weeks|18.00|TRUE
B|4 weeks|32.00|FALSE
D|8 years|399.99|TRUE
P|8 years|299.99|FALSE
B|8 years|1099.99|TRUE
D|3 years|199.00|TRUE
B|3 years|450.00|FALSE
P|3 years|250.00|TRUE
D|6 months|40.00|FALSE
P|6 months|55.00|TRUE
B|6 months|100.00|FALSE
D|3 months|25.00|TRUE
P|3 months|30.00|FALSE
B|3 months|60.00|TRUE
\.

-- Editions(editionDate, numArticles, numPages)
COPY Editions FROM stdin USING DELIMITERS '|';
2022-12-01|3|5
2022-06-01|10|15
2022-03-01|5|23
2022-01-01|13|67
2021-02-15|3|10
2021-04-01|3|3
2021-06-13|25|25
2023-08-30|2|30
2021-06-15|20|40
2021-10-15|10|34
\.

-- Subscribers(subscriberPhone, subscriberName, subscriberAddress)
COPY Subscribers FROM stdin USING DELIMITERS '|';
8315512|Steve Rogers|300D Quincy Street, New York, NY, 10213
6505523|Cersei Lannister|3428A Lombard St, Tahoe City, CA, 96142
4155534|Doctor Strange|300D Quincy Street, New York, NY, 10213
5105545|Carol Danvers|300D Quincy Street, New York, NY, 10213
9255556|Tony Stark|3428A Lombard St, Tahoe City, CA, 96142
8315567|Bruce Banner|3428A Lombard St, Tahoe City, CA, 96142
6502123|Doctor Strange|13 Bleecker St, New York, NY, 10021
8313293|Thor Odinson|100 Asgard St, Asgard, AG, 00001
9201342|Kakashi Sensei|300 Hokage Rock, Konohagakure, 23104
8742311|Light Yagami|325 Daikoku Gauken, Tokyo, JP, 35324
7482912|Jon Snow|0001 Sector 52, The Hole in the Wall Caffe, MD, 64732
7572383|Sansa Stark|757 North First St, Winterfell, 78643
2636123|Robert Baratheon|333 Amelia St, Kings Landing, 34817\
\.

-- Subscriptions(subscriberPhone, subscriptionStartDate, subscriptionMode, subscriptionInterval, paymentReceived)
COPY Subscriptions FROM stdin USING DELIMITERS '|';
8315512|2023-01-01|D|1 years|FALSE
6505523|2022-12-15|D|1 years|TRUE
5105545|2022-04-01|D|1 years|TRUE
8315567|2023-03-01|D|1 years|TRUE
6502123|2022-12-10|D|1 years|FALSE
8313293|2022-10-01|D|1 years|TRUE
8315512|2022-12-01|B|1 years|FALSE
6505523|2022-12-27|B|1 years|FALSE
8315567|2023-09-01|B|1 years|TRUE
6502123|2022-12-06|B|1 years|FALSE
6502123|2023-04-15|B|1 years|FALSE
4155534|2022-12-08|P|1 years|TRUE
9255556|2023-06-02|P|1 years|FALSE
9201342|2022-12-31|P|1 years|TRUE
9255556|2022-08-03|D|4 weeks|FALSE
4155534|2022-09-03|D|4 weeks|FALSE
8742311|2023-10-03|D|4 weeks|FALSE
8742311|2023-11-03|D|4 weeks|FALSE
4155534|2021-09-03|P|4 weeks|FALSE
8742311|2021-10-03|P|4 weeks|FALSE
8742311|2022-11-03|P|4 weeks|FALSE
7482912|2019-01-01|B|4 weeks|TRUE
7572383|2019-02-01|B|4 weeks|FALSE
7482912|2018-09-01|D|8 years|TRUE
7572383|2018-03-02|D|8 years|TRUE
7482912|2017-05-01|P|8 years|FALSE
7572383|2017-03-02|P|8 years|TRUE
7482912|2012-05-01|D|3 years|FALSE
7572383|2012-03-02|D|3 years|TRUE
6502123|2022-01-19|B|8 years|FALSE
2636123|2014-04-30|B|3 years|FALSE
2636123|2014-06-20|P|3 years|FALSE
2636123|2015-04-30|D|6 months|FALSE
2636123|2016-04-30|B|6 months|FALSE
2636123|2017-04-30|P|6 months|FALSE
2636123|2018-04-30|D|3 months|FALSE
\.

-- Holds(subscriberPhone, subscriptionStartDate, holdStartDate, holdEndDate)
COPY Holds FROM stdin USING DELIMITERS '|';
6502123|2022-12-06|2023-01-01|2023-11-01
6505523|2022-12-27|2023-01-31|2023-02-28
6505523|2022-12-27|2023-03-31|2023-04-30
5105545|2022-04-01|2022-10-01|2022-12-01
5105545|2022-04-01|2023-01-21|2023-01-21
5105545|2022-04-01|2022-04-01|2023-05-10
8313293|2022-10-01|2022-10-01|2022-10-01
\.

-- Articles(editionDate, articleNum, articleAuthor, articlePage)
COPY Articles FROM stdin USING DELIMITERS '|';
2022-12-01|4|Green Goblin|10
2022-12-01|6|Doctor Strange|7
2022-12-01|1|Doctor Strange|4
2021-02-15|1|Cersei Lannister|1
2021-02-15|2|Cersei Lannister|12
2021-02-15|3|Cersei Lannister|15
2021-04-01|1|Ragnar Lothbrok|1
2021-04-01|2|Ragnar Lothbrok|2
2021-02-15|5|Ragnar Lothbrok|4
2021-06-13|1|Ragnar Lothbrok|1
2021-06-13|2|Ragnar Lothbrok|1
2022-12-01|2|Ragnar Lothbrok|2
2022-06-01|1|Ragnar Lothbrok|10
2022-03-01|1|Ragnar Lothbrok|4
2022-01-01|1|Ragnar Lothbrok|3
2022-01-01|4|Ragnar Lothbrok|3
2022-01-01|5|Ragnar Lothbrok|3
2021-04-01|3|Judith|4
2021-02-15|6|Judith|3
2021-06-13|3|Judith|4
2021-06-13|4|Judith|3
2021-06-15|1|Judith|1
2022-06-01|2|Lucius Malfoy|11
2022-06-01|3|Lucius Malfoy|11
2022-06-01|4|Lucius Malfoy|11
2022-03-01|2|Lucius Malfoy|13
2022-01-01|3|Lucius Malfoy|2
2022-03-01|3|Lucius Malfoy|2
2021-02-15|9|Lucius Malfoy|1
2021-06-13|10|Lucius Malfoy|3
2021-04-01|4|Lucius Malfoy|10
2023-08-30|1|Lucius Malfoy|1
2023-08-30|2|Barty Crouch Jr.|3
2023-08-30|3|Gregor Clegane|5
2023-08-30|4|Bellatrix Lestrange|10
\.

-- ReadArticles(subscriberPhone, editionDate, articleNum, readInterval)
COPY ReadArticles FROM stdin USING DELIMITERS '|';
8315512|2021-02-15|1|00:15:00
8315512|2021-04-01|1|00:25:36
8315512|2023-08-30|3|00:00:01
5105545|2023-08-30|3|00:19:03
6505523|2022-12-01|4|01:45:02
6505523|2022-12-01|6|00:00:05
6505523|2023-08-30|2|00:14:32
6505523|2023-08-30|1|22:20:32
8315567|2021-02-15|1|10:32:30
8315567|2021-02-15|2|00:03:23
8315567|2021-02-15|5|00:10:32
8315567|2023-08-30|1|00:12:34
6502123|2022-12-01|4|00:07:30
6502123|2022-12-01|1|00:08:45
6502123|2021-02-15|1|00:09:31
6502123|2021-02-15|2|00:06:53
6502123|2021-02-15|5|10:07:11
6502123|2021-04-01|1|00:14:37
6502123|2023-08-30|1|00:23:25
6502123|2023-08-30|2|00:06:41
6502123|2023-08-30|3|08:08:08
8313293|2023-08-30|3|00:01:00
\.
