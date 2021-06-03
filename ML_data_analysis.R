library(caret)
library(neuralnet)
#setwd("C:/Users/anssi/Desktop/Git/space-greeter") # if on windows
setwd("/home/anssi/Git/space-greeter")
log_file<-"log"
# Load test_data and fix broken values
test_data <- read.csv(paste(log_file,".csv",sep=""), header = TRUE, sep=";")
#Change this
test_data[is.na(test_data)]<-0
test_data$FPS[which(!is.finite(test_data$FPS))]<-0
test_data$FPS[which(test_data$FPS>60)]<-60
write.csv(test_data,"test_data.csv")

# MAX-MIN NORMALIZATION
normalize <- function(x) {
  return ((x - min(x)) / (max(x) - min(x)))
}


dataset<-"dup1"#"dup1" #dup1/dup2/fb
single<-1
radius<--1
neighbours<--1
scale<--1
cascade_number<--1
  #1 lbpcascades/lbpcascade_frontalface.xml
  #2 lbpcascades/lbpcascade_frontalface_improved.xml
  #3 haarcascades/haarcascade_frontalface_default.xml
  #4 haarcascades/haarcascade_frontalface_alt.xml
  #5 haarcascades/haarcascade_frontalface_alt2.xml
  #6 haarcascades/haarcascade_frontalface_alt_tree.xml
test_data$Cascade[which(test_data$Cascade=="lbpcascades/lbpcascade_frontalface.xml")]<-as.numeric(1)
test_data$Cascade[which(test_data$Cascade=="lbpcascades/lbpcascade_frontalface_improved.xml")]<-as.numeric(2)
test_data$Cascade[which(test_data$Cascade=="haarcascades/haarcascade_frontalface_default.xml")]<-as.numeric(3)
test_data$Cascade[which(test_data$Cascade=="haarcascades/haarcascade_frontalface_alt.xml")]<-as.numeric(4)
test_data$Cascade[which(test_data$Cascade=="haarcascades/haarcascade_frontalface_alt2.xml")]<-as.numeric(5)
test_data$Cascade[which(test_data$Cascade=="haarcascades/haarcascade_frontalface_alt_tree.xml")]<-as.numeric(6)
  
# Filter to include only selected dataset(s)
if(dataset!=-1){
  test_data <-test_data[test_data$Dataset %in% c(dataset),]
}
test_data <- test_data[c("Neighbours","Radius","Scale","Cascade","Accuracy","FPS")]
test_data[,] <- sapply(test_data[,], as.numeric)
original_data<-test_data
test_data <- as.data.frame(lapply(test_data, normalize))
#test_data<-transform(test_data,Accuracy = (Accuracy - min(Accuracy))/ (max(Accuracy) - min(Accuracy)))
#test_data<-transform(test_data,FPS = (FPS - min(FPS))/ (max(FPS) - min(FPS)))
#test_data<-transform(test_data,Cascade = (Cascade - min(Cascade))/ (max(Cascade) - min(Cascade)))


test_data$eval<-test_data$Accuracy*test_data$FPS

#Change this
test_data$Measured <- test_data$Accuracy
original_data$Measured <- test_data$Accuracy

test_data <- as.data.frame(lapply(test_data, normalize))
cutoff<-floor(nrow(test_data)*0.8)
trainset <- test_data[1:cutoff,]
testset <- test_data[(cutoff+1):nrow(test_data), ]

nn <- neuralnet(Measured ~ Neighbours + Radius + Scale + Cascade, data=trainset, hidden=c(20,10), linear.output=TRUE, threshold=0.01)
nn$result.matrix
plot(nn)

nn.results <-compute(nn,testset)
results <- data.frame(actual = testset$Measured, prediction = nn.results$net.result)
results

predicted=results$prediction* abs(diff(range(original_data$Measured))) + min(original_data$Measured)
actual=results$actual* abs(diff(range(original_data$Measured))) + min(original_data$Measured)
comparison=data.frame(predicted,actual)
deviation=abs(((actual-predicted)/actual))
comparison=data.frame(predicted,actual,deviation)
accuracy=1-abs(mean(deviation))
accuracy

Neig <- seq(1, 10, length.out = 10)
Rad <- seq(1, 30, length.out = 30)
Sca <- seq(1, 10, length.out = 40)
Cas <- seq(1,6, length.out = 6)
d1 <- expand.grid(Neighbours = Neig, Radius = Rad, Scale = Sca, Cascade = Cas)
d1_normalized <- as.data.frame(lapply(d1, normalize))
nn.results <- compute(nn,d1)
d1_normalized$eval <- nn.results$net.result
d1$Neighbours <- d1_normalized$Neighbours* abs(diff(range(d1$Neighbours))) + min(d1$Neighbours)
d1$Radius <- d1_normalized$Radius* abs(diff(range(d1$Radius))) + min(d1$Radius)
d1$Scale <- d1_normalized$Scale* abs(diff(range(d1$Scale))) + min(d1$Scale)
d1$Cascade <- d1_normalized$Cascade* abs(diff(range(d1$Cascade))) + min(d1$Cascade)
d1$eval <- d1_normalized$eval
