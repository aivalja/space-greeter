#import data somehow to _data_
library(readxl)
data <- read_excel("test_data.xlsx", sheet ="All results combined")
#Change this
data<-transform(data,Accuracy...4 = (Accuracy...4 - min(Accuracy...4))/ (max(Accuracy...4) - min(Accuracy...4)))
data<-transform(data,FPS...1 = (FPS...1 - min(FPS...1))/ (max(FPS...1) - min(FPS...1)))
data$eval<-data$Accuracy...4*data$FPS...1
data$Neighbours<-data$Neighbours...3
data$Radius<-data$Radius...2

a <- data$Neighbours...3
a_string<-"Neighbours"
name_of_file<-"chart_Neighbours_vs_evaluation_formula.svg"
x<-a
degree<-3

y<-data$eval
xx <- seq(min(a),max(a), length.out=293)
fit <- lm( y~poly(x,degree) )
library(ggplot2)
library(plyr)
data_summary <- function(data, varname, groupnames){
  require(plyr)
  summary_func <- function(x, col){
    c(mean = mean(x[[col]], na.rm=TRUE),
      sd = sd(x[[col]], na.rm=TRUE))
  }
  data_sum<-ddply(data, groupnames, .fun=summary_func,
                  varname)
  data_sum <- rename(data_sum, c("mean" = varname))
  return(data_sum)
}

df2 <- data_summary(data,varname="eval",groupnames=c(a_string))
avg <- df2$eval
sdev <- df2$sd
# Change this
long <- df2$Neighbours

svg(filename=name_of_file, width=6.75, height=5)
plot(long, avg,
     ylim=range(c(0, avg+sdev)),
     pch=19, xlab=a_string, ylab="Evaluation formula +/- SD",
     xaxt = "n",
     yaxt = "n"
)
lines(xx, predict(fit, data.frame(x=xx)), col='black')

axis(1, at=0:max(a), tck=0.02)
rug(x=(2:(max(a)*2))/2, ticksize = 0.01,side=1)

axis(2, at=(0:round(max(avg+sdev)*10))/10, tck=0.02)
rug(x=(0:round(max(avg+sdev)*50))/50, ticksize = 0.01,side=2)

arrows(long, avg-sdev, long, avg+sdev, length=0.05, angle=90, code=3)
r2<-summary(fit)$r.squared
mylabel = bquote(italic(R)^2 == .(format(r2, digits = 3)))
legend('topright', legend=mylabel, bty ='n')
dev.off()

