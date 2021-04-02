setwd("C:/Users/anssi/Desktop/Git/space-greeter") # if on windows
log_file<-"log"
dataset<-"dup1" #dup1/dup2/fb
single<-FALSE
cascade_number<-1
  #1 lbpcascades/lbpcascade_frontalface.xml
  #2 lbpcascades/lbpcascade_frontalface_improved.xml
  #3 haarcascades/haarcascade_frontalface_default.xml
  #4 haarcascades/haarcascade_frontalface_alt.xml
  #5 haarcascades/haarcascade_frontalface_alt2.xml
  #6 haarcascades/haarcascade_frontalface_alt_tree.xml
temp_cascades <- list("lbpcascades/lbpcascade_frontalface.xml","lbpcascades/lbpcascade_frontalface_improved.xml","haarcascades/haarcascade_frontalface_default.xml","haarcascades/haarcascade_frontalface_alt.xml","haarcascades/haarcascade_frontalface_alt2.xml","haarcascades/haarcascade_frontalface_alt_tree.xml")
cascade<- temp_cascades[cascade_number]
column_compared<-"FPS" #Neighbours/Radius/Scale
y_axis_column<-"Accuracy" #Accuracy/FPS/eval/Detect.Accuracy

#if(single){
#  folder<-"single_face"
#} else {
#  folder<-"multiple_faces"
#}
folder<-"multiple_faces_cascades"

data <- read.csv(paste("test_results/",folder,"/",log_file,".csv",sep=""), header = TRUE, sep=";")
#Change this
data[is.na(data)]<-0
data$FPS[which(!is.finite(data$FPS))]<-0
#data$Detect.Accuracy<-as.numeric(sub("%","",data$Detect.Accuracy))/100
#data<-transform(data,Accuracy = (Accuracy - min(Accuracy))/ (max(Accuracy) - min(Accuracy)))
#data<-transform(data,FPS = (FPS - min(FPS))/ (max(FPS) - min(FPS)))
#data$eval<-data$Accuracy*data$FPS
write.csv(data,"data.csv")
# THis part only for testing ####
data2<-data[data$Scale==1.0,]
plot(data2[[column_compared]], data2[[y_axis_column]], col="blue",
     pch=19, xlab=column_compared, ylab=paste0(y_axis_column)
)
# Make a loop using something like this for color?
data3<-data[data$Scale==1.1,]
points(data3[[column_compared]], data3[[y_axis_column]], col=rgb(1/1.1,1/1.1,1/1.1),
     pch=19, xlab=column_compared, ylab=paste0(y_axis_column)
)

data3<-data[data$Scale==1.2,]
points(data3[[column_compared]], data3[[y_axis_column]], col="green",
       pch=19, xlab=column_compared, ylab=paste0(y_axis_column)
)

data3<-data[data$Scale==1.5,]
points(data3[[column_compared]], data3[[y_axis_column]], col="green",
       pch=19, xlab=column_compared, ylab=paste0(y_axis_column)
)

data3<-data[data$Scale==1.8,]
points(data3[[column_compared]], data3[[y_axis_column]], col="green",
       pch=19, xlab=column_compared, ylab=paste0(y_axis_column)
)
######

a <- data[[column_compared]]
a_string<-column_compared
#name_of_file<-paste("chart_",column_compared,"_vs_eval_",folder,".svg",sep="")
x<-a
degree<-min(3,length(unique(data[[column_compared]]))-1)

y<-data[[y_axis_column]]
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

df2 <- data_summary(data,varname=y_axis_column,groupnames=c(a_string))
avg <- df2[[y_axis_column]]
sdev <- df2$sd
# Change this
long <- df2[[column_compared]]

#svg(filename=name_of_file, width=6.75, height=5)
plot(long, avg,
     ylim=range(c(0, avg+sdev)),
     pch=19, xlab=a_string, ylab=paste0(y_axis_column," formula +/- SD"),
     xaxt = "n",
     yaxt = "n"
)
lines(xx, predict(fit, data.frame(x=xx)), col='black')

axis(1, at=0:max(a), tck=0.02)
#rug(x=(2:(max(a)*2))/2, ticksize = 0.01,side=1)

axis(2, at=(0:round(max(avg+sdev)*1000))/1000, tck=0.0002)
rug(x=(0:round(max(avg+sdev)*500))/500, ticksize = 0.0001,side=2)

arrows(long, avg-sdev, long, avg+sdev, length=0.05, angle=90, code=3)
r2<-summary(fit)$r.squared
mylabel = bquote(italic(R)^2 == .(format(r2, digits = 3)))
legend('topright', legend=mylabel, bty ='n')
#dev.off()

