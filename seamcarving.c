//Seam Carving
//Aahan Madhok and Leo Gurevitch 
//April, 2024
#include <stdio.h>
#include "c_img.h"
#include <math.h>

//Rx(y,x)=(y,x+1)red−(y,x−1)red

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    // Initialize the grad image
    create_img(grad, im->height, im->width);
    for(int y = 0; y < im->height; y++){    
        for(int x = 0; x < im->width; x++){
            int rx, gx, bx, ry, gy, by;
            //Set up variables for how far to go in each direction from the pixel
            //to calculate the energy, accounting for the edge cases:
            int x_left;
            int y_down;
            int x_right;
            int y_up;
            x_left = x - 1;
            y_down = y - 1;
            x_right = x + 1;
            y_up = y + 1;
            if (x + 1 == im->width){
                x_right = 0;
            }
            if (x == 0){
                x_left = im->width - 1;
            }
            if (y + 1 == im->height){
                y_up = 0;
            }
            if (y == 0){
                y_down = im->height - 1;
            }

             rx = get_pixel(im, y, x_right, 0) - get_pixel(im, y, x_left, 0);
             gx = get_pixel(im, y, x_right, 1) - get_pixel(im, y, x_left, 1); 
             bx = get_pixel(im, y, x_right, 2) - get_pixel(im, y, x_left, 2);

             ry = get_pixel(im, y_up, x, 0) - get_pixel(im, y_down, x, 0);
             gy = get_pixel(im, y_up, x, 1) - get_pixel(im, y_down, x, 1);
             by = get_pixel(im, y_up, x, 2) - get_pixel(im, y_down, x, 2);
            

            int delta_x_squared = (rx*rx) + (gx*gx) + (bx*bx);
            int delta_y_squared = (ry*ry) + (gy*gy) + (by*by);

            uint8_t energy = (sqrt(delta_x_squared + delta_y_squared)) / 10; 
            set_pixel(*grad, y, x, energy, energy, energy);


        }
    }
    


    

}

double minimum(double a, double b, double c){
    if(a < b && a<c){
        return a;
    }
    if(b < c){
        return b;
    }
    else{
        return c;
    }
}

double min_edge(double a, double b)
{
    if (a < b){
        return a;
    } else{
        return b;
    }
}


void dynamic_seam(struct rgb_img *grad, double **best_arr){

    *best_arr = (double*)malloc(sizeof(double)*(grad->width)*(grad->height));
    for(int j = 0; j < grad->height; j++){
        for(int i = 0; i < grad->width; i++){
            
            if(j == 0){
                (*best_arr)[i] = grad->raster[i*3];
            }

// grad->width == (*(grad+width))
            else{

                if(i != 0 && i < grad->width - 1){
                    (*best_arr)[j*(grad->width)+i] = grad->raster[3*(j*(grad->width)+i)] + minimum((*best_arr)[(j-1)*(grad->width) + i], (*best_arr)[(j-1)*(grad->width) + (i+1)], (*best_arr)[(j-1)*(grad->width) + (i-1)]);
                }

                if(i == 0){
                    (*best_arr)[j*(grad->width)+i] = grad->raster[3*(j*(grad->width)+i)] + min_edge((*best_arr)[(j-1)*(grad->width) + i], (*best_arr)[(j-1)*(grad->width) + (i+1)]);
                    }

                if(i == grad->width - 1){
                    (*best_arr)[j*(grad->width)+i] = grad->raster[3*(j*(grad->width)+i)] + min_edge((*best_arr)[(j-1)*(grad->width) + i], (*best_arr)[(j-1)*(grad->width) + (i-1)]);
                    }

            }
      

        }   
    }
}

void recover_path(double *best, int height, int width, int **path)
{
    (*path) = (int *)malloc(height * sizeof(int));
    //Find the cheapest overall path at the bottom:
    int cur_min_bottom = best[(height - 1) * width]; //The first element in the bottom row of
    //the best array. Start that as the minimum.
    for (int i = 1; i < width; i++){
        if (best[(height - 1) * width + i] < cur_min_bottom){
            cur_min_bottom = best[(height - 1) * width + i];
            (*path)[height - 1] = i; //Set the last element in the path array to the cheapest
            //'cost' in the bottom row.
        }
    } 
    int cur_index = (*path)[height - 1];
    int check_row;
    int cur_min;
    int cur_min_index;
    for (int i = height - 1; i > 0; i--){//Start from the bottom and work upwards until the row before
    //the top row, which is the last row to check (since we will check the rows above the current row).
        check_row = (i - 1) * width;
        if (cur_index == 0){//Left-side edge case.
            cur_min = best[check_row + cur_index];
            cur_min_index = cur_index;
            if (best[check_row + cur_index + 1] < cur_min){
                cur_min = best[check_row + cur_index + 1]; //I know this is kind of unnecessary,
                //but I added it for consistency with my code below.
                cur_min_index = cur_index + 1;
            }
        }
        else if (cur_index == width - 1){//Right-side edge case.
            cur_min = best[check_row + cur_index];
            cur_min_index = cur_index; //I realized after coding this that combined with the step of resetting
            //cur_min_index in each iteration, this part is unnecessary.
            if (best[check_row + cur_index - 1] < cur_min){
                cur_min = best[check_row + cur_index - 1]; //I know this is kind of unnecessary,
                //but I added it for consistency with my code below.
                cur_min_index = cur_index - 1;
            }
        }
        else{
            cur_min = best[check_row + cur_index - 1];
            cur_min_index = cur_index - 1;
            if (best[check_row + cur_index] < cur_min){
                cur_min = best[check_row + cur_index];
                cur_min_index = cur_index;
            }
            if (best[check_row + cur_index + 1] < cur_min){
                cur_min = best[check_row + cur_index + 1];
                cur_min_index = cur_index + 1;
            }
        }
        cur_index = cur_min_index;
        (*path)[i - 1] = cur_index;
        cur_min_index = -1000;
        cur_min = -1000; 
    }

}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path)
{
    create_img(dest, src->height, src->width - 1); //Width - 1 since we are removing a vertical seam.
    for (int i = 0; i < src->height; i++){
        
        for (int j = 0; j < path[i]; j++){
            uint8_t r = get_pixel(src, i, j, 0);
            uint8_t g = get_pixel(src, i, j, 1);
            uint8_t b = get_pixel(src, i, j, 2);
            set_pixel((*dest), i, j, r, g, b);   
        }
        for (int k = path[i] + 1; k < src->width; k++){
            uint8_t r = get_pixel(src, i, k, 0);
            uint8_t g = get_pixel(src, i, k, 1);
            uint8_t b = get_pixel(src, i, k, 2);
            set_pixel((*dest), i, k - 1, r, g, b);
            //When setting the pixel in dest, we use k-1 since we skipped a pixel in src, and dest will
            //have a width that is one less than src.
        }
    }
    
}

// int main(){
    //Our testing code while coding this project:
    // struct rgb_img *im;
    // char* filename = "6x5.bin";
    // read_in_img(&im, filename);
    // struct rgb_img *grad;
    // calc_energy(im, &grad);    
    // print_grad(grad);

    // double *best_arr;
    // dynamic_seam(grad, &best_arr); // Pass the address of best_arr
    // for(int j = 0; j < grad->height; j++){
    //     for(int i = 0; i < grad->width; i++){
    //         printf("%f ", best_arr[j*(grad->width)+i]);
    // }
    // printf("\n");
    // }

    // int *path;
    // recover_path(best_arr, grad->height, grad->width, &path);
    // for (int i = 0; i < grad->height; i++){
    //     printf("%d, ", path[i]);
    // }

    //Testing code

    // struct rgb_img *im;
    // struct rgb_img *cur_im;
    // struct rgb_img *grad;
    // double *best;
    // int *path;

    // read_in_img(&im, "HJoceanSmall.bin");
    
    // for(int i = 0; i < 5; i++){
    //     printf("i = %d\n", i);
    //     calc_energy(im,  &grad);
    //     dynamic_seam(grad, &best);
    //     recover_path(best, grad->height, grad->width, &path);
    //     remove_seam(im, &cur_im, path);

    //     char filename[200];
    //     sprintf(filename, "img%d.bin", i);
    //     write_img(cur_im, filename);


    //     destroy_image(im);
    //     destroy_image(grad);
    //     free(best);
    //     free(path);
    //     im = cur_im;
    // }
    // destroy_image(im);

    //More of our own testing code:
    // struct rgb_img *test_1;
    // struct rgb_img *cur_im_test_1;
    // struct rgb_img *grad_test_1;
    // double *best_test_1;
    // int *path_test_1;
    // read_in_img(&test_1, "HJoceanSmall.bin");
    // calc_energy(test_1, &grad_test_1);
    // dynamic_seam(grad_test_1, &best_test_1);
    // recover_path(best_test_1, grad_test_1->height, grad_test_1->width, &path_test_1);
    // remove_seam(test_1, &cur_im_test_1, path_test_1);
    // test_1 = cur_im_test_1;
    // printf("i = 0\n");
    // for(int i = 1; i < 251; i++){
    //     printf("i = %d\n", i);
    //     calc_energy(test_1, &grad_test_1);
    //     dynamic_seam(grad_test_1, &best_test_1);
    //     recover_path(best_test_1, grad_test_1->height, grad_test_1->width, &path_test_1);
    //     remove_seam(test_1, &cur_im_test_1, path_test_1);
    //     test_1 = cur_im_test_1;
    // }
    // char filename_test_1[200];
    // strcpy(filename_test_1, "test_250_seams_removed.bin");
    // write_img(cur_im_test_1, filename_test_1);
    // destroy_image(test_1);
    // destroy_image(grad_test_1);
    // free(best_test_1);
    // free(path_test_1);
    //Success!

    //Testing removing 506 seams from an image of width 507:
    // struct rgb_img *test_1;
    // struct rgb_img *cur_im_test_1;
    // struct rgb_img *grad_test_1;
    // double *best_test_1;
    // int *path_test_1;
    // read_in_img(&test_1, "HJoceanSmall.bin");
    // calc_energy(test_1, &grad_test_1);
    // dynamic_seam(grad_test_1, &best_test_1);
    // recover_path(best_test_1, grad_test_1->height, grad_test_1->width, &path_test_1);
    // remove_seam(test_1, &cur_im_test_1, path_test_1);
    // test_1 = cur_im_test_1;
    // printf("i = 0\n");
    // for(int i = 1; i < 506; i++){
    //     printf("i = %d\n", i);
    //     calc_energy(test_1, &grad_test_1);
    //     dynamic_seam(grad_test_1, &best_test_1);
    //     recover_path(best_test_1, grad_test_1->height, grad_test_1->width, &path_test_1);
    //     remove_seam(test_1, &cur_im_test_1, path_test_1);
    //     test_1 = cur_im_test_1;
    // }
    // char filename_test_1[200];
    // strcpy(filename_test_1, "test_506_seams_removed.bin");
    // write_img(cur_im_test_1, filename_test_1);
    // destroy_image(test_1);
    // destroy_image(grad_test_1);
    // free(best_test_1);
    // free(path_test_1);
    //Success!

    //Testing with a high-resolution photo:
    // struct rgb_img *test_1;
    // struct rgb_img *cur_im_test_1;
    // struct rgb_img *grad_test_1;
    // double *best_test_1;
    // int *path_test_1;
    // read_in_img(&test_1, "moraine_lake.bin"); //Testing it with a high-resolution photo of Moraine Lake
    //looked like it worked well! The image still looked normal (nothing weird happening, no blank spaces,
    //etc.) - just compressed, but in a nice way so as not to diminish quality too much! As expected and hoped for!

    // calc_energy(test_1, &grad_test_1);
    // dynamic_seam(grad_test_1, &best_test_1);
    // recover_path(best_test_1, grad_test_1->height, grad_test_1->width, &path_test_1);
    // remove_seam(test_1, &cur_im_test_1, path_test_1);
    // test_1 = cur_im_test_1;
    // destroy_image(grad_test_1);
    // free(best_test_1);
    // free(path_test_1);
    // printf("i = 0\n");
    // for(int i = 1; i < 251; i++){
    //     printf("i = %d\n", i);
    //     calc_energy(test_1, &grad_test_1);
    //     dynamic_seam(grad_test_1, &best_test_1);
    //     recover_path(best_test_1, grad_test_1->height, grad_test_1->width, &path_test_1);
    //     remove_seam(test_1, &cur_im_test_1, path_test_1);
    //     test_1 = cur_im_test_1;
    //     destroy_image(grad_test_1);
    //     free(best_test_1);
    //     free(path_test_1);
    // }
    // char filename_test_1[200];
    // strcpy(filename_test_1, "test_high_res_photo.bin");
    // write_img(cur_im_test_1, filename_test_1);
    // destroy_image(test_1);
    // destroy_image(grad_test_1);
    // free(best_test_1);
    // free(path_test_1);
// }