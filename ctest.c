#include<stdio.h>
#include<ctype.h>

int main(){
    
    char figure;
    const float PI = 3.14159276;
    float perimetre, surface, rayon, longueur, largeur, cote;
    

    printf("Veuillez saisir le code d'une figure (c, r, k) : ");
    scanf("%c", &figure);

    switch (toupper(figure)){
        case 'C': 
            printf("Veuillez saisir le rayon du cercle: ");
            scanf("%f", &rayon);
            perimetre = 2 * PI * rayon;
            surface = PI * (rayon * rayon);
            break;
        case 'R':
            printf("Veuillez saisir la longueur puis la largeur du rectangle, separes par un espace: ");
            scanf("%f %f", &longueur, &largeur);
            perimetre = 2 * (longueur + largeur);
            surface = longueur * largeur;
            break;
        case 'K':
            printf("Veuillez saisir la longueur du cote du carre: ");
            scanf("%f", &cote);
            perimetre = 4 * cote;
            surface = cote * cote;
            break;
        default: 
            printf("Ceci n'est pas un code de forme");
            return 1;

    }

    printf("Perimetre = %.2f, Surface = %.2f.", perimetre, surface);

    return 0;
    
}

