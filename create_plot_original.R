dataset<-"dup1" #dup1/dup2/fb/all
single<-TRUE
column_compared<-"Radius" #Neighbours/Radius

if(single){
  folder<-"single_face"
} else {
  folder<-"multiple_faces"
}

data <- read.csv(paste("test_results/",folder,"/log_",dataset,".csv",sep=""), header = TRUE, sep=";")
#Change this
data$Accuracy<-as.numeric(sub("%","",data$Accuracy))/100
data$Detect.Accuracy<-as.numeric(sub("%","",data$Detect.Accuracy))/100
data<-transform(data,Accuracy = (Accuracy - min(Accuracy))/ (max(Accuracy) - min(Accuracy)))
data<-transform(data,FPS = (FPS - min(FPS))/ (max(FPS) - min(FPS)))
data$eval<-data$Accuracy*data$FPS

a <- data[[column_compared]]
a_string<-column_compared
name_of_file<-paste("chart_",column_compared,"_vs_eval_",folder,".svg",sep="")
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
long <- df2[[column_compared]]

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

