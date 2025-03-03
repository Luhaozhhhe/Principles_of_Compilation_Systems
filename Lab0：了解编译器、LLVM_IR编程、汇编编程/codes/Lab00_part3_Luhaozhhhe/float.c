#include <stdio.h>  
int main() {  
    float num1, num2;  
    printf("please input the first float number:");  
    scanf("%f", &num1);  
    printf("please input the second float number: ");  
    scanf("%f", &num2);  

    if (num1 > num2) {  
    
        printf("%.2f is bigger than %.2f\n", num1, num2);  
    } 
    else {  
    
        printf("%.2f is not bigger than %.2f\n", num1, num2);  
    }  
  
    return 0;  
}
