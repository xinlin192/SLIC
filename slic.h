/*###############################################################
## MODULE: slic.cpp
## VERSION: 1.0 
## SINCE 2013-12-17
## AUTHOR Jimmy Lin (xl5224) - JimmyLin@utexas.edu  
## DESCRIPTION: 
##      
#################################################################
## Edited by MacVim
## Class Info auto-generated by Snippet 
################################################################*/

#include "stdlib.h"
#include "math.h"
#include "cv.h"

#define TEST

/* Structure keeping track of info of the cluster center */
struct centroid 
{
    int x, y;
    double l, a, b;
}

/* Update the centroid */
void update_centroid (struct centroid * c, double l, double a, double b, int x, int y)
{
    c->l = l;
    c->a = a;
    c->b = b;
    c->x = x;
    c->y = y;
    return ;
}

/* Assist to provide valid point position */
int validate (int value, int lower, int upper)
{
    if (value < lower) return lower;
    else if (value >= upper) return upper;
    else return value;
}

/* 
 * INPUT
 *   imgLab: image matrix in EILAB format
 *   k: number of superpixel to generate
 */
void slic (cv::Mat imgLab, const int k, double threshold) 
{
    const int H = imgLab.rows;
    const int W = imgLab.cols;

    // randomly pick up initial cluster center
    const int S = sqrt(H * W / k);  // grid size
    const int gridPerRow = W / S;
    vector<centroid *> ccs = vector(k, NULL);
    for (int i = 0; i < k; i ++) {
        // randomize the position of centroid
        int x, y;
        double l, a, b;
        gridx = i % gridPerRow;
        gridy = i / gridPerRow;
        x = (rand() % S) + gridx * S;
        y = (rand() % S) + gridy * S;
        // acquire lab color of the derived centroid
        l = imgLab.at<double>(y, x)[0];
        a = imgLab.at<double>(y, x)[1];
        b = imgLab.at<double>(y, x)[2];
        // create centroid object
        ccs[i] = (struct centroid *) malloc (sizeof (struct centroid));
        // update the centroid
        update_centroid (ccs[i], l, a, b, x, y);
    }

    // TODO: move cluster center to the lowest gradient position
     

    // label matrix indicate the cluster number one pixel is in
    cv::Mat label = cv::Mat (H, W, CV_8U, -1);
    // distance matrix represents the distance between one pixel and its
    // centroid
    cv::Mat distance = cv::Mat (H, W, CV_64F, INFINITY);

    while (true) 
    {
        for (int i = 0; i < k; i ++)
        {
            // acquire labxy attribute of centroid
            int x = ccs[i]->x;
            int y = ccs[i]->y;
            double l = ccs[i]->l;
            double a = ccs[i]->a;
            double b = ccs[i]->b;

            // look around its 2S x 2S region
            for (int tmpx = validate(x-S, 0, W); tmpx < validate(x+S, 0, W); tmpx++) 
            {
                for (int tmpy = validate(y-S, 0, H), tmpy < validate(y+S, 0, H), tmpy++) {
                    double tmpl = imgLab.at<double>(tmpy, tmpx)[0];
                    double tmpa = imgLab.at<double>(tmpy, tmpx)[1];
                    double tmpb = imgLab.at<double>(tmpy, tmpx)[2];

                    double color_distance = sqrt (pow(tmpl - l, 2.0) +
                            pow(tmpa - a, 2.0) + pow(tmpb, 2.0)); 
                    double spatial_distance = sqrt (pow(tmpx - x, 2.0) +
                            pow(tmpy - y, 2.0));

                    // FIXME: to be refined formula for ultimate distance
                    double D = sqrt (pow(color_distance, 2.0) +
                            pow(spatial_distance / S, 2.0));

                    // distance is smaller, update the centroid it belong to
                    if (D < distance.at<double>(tmpy, tmpx)) {
                        distance.at<double>(tmpy, tmpx) = D;
                        label.at<unsigned>(tmpy, tmpx) = k;
                    }
                }

            }
        }

        // Compute new cluster center by taking the mean of each dimension
        vector<int> count = vector (k, 0);
        vector<double> sumx = vector (k, 0.0);
        vector<double> sumy = vector (k, 0.0);
        vector<double> suml = vector (k, 0.0);
        vector<double> suma = vector (k, 0.0);
        vector<double> sumb = vector (k, 0.0);

        for (int y = 0; y < H; y++) 
        {
            for (int x = 0; x < W; x++)
            {
                unsigned clusterIdx = label.at<unsigned>(y, x);
                count[clusterIdx] ++;
                sumx[clusterIdx] += x;
                sumy[clusterIdx] += y;
                suml[clusterIdx] += l;
                suma[clusterIdx] += a;
                sumb[clusterIdx] += b;
            }
        }

        vector<centroid *> newccs = vector (k, NULL);
        for (int i = 0; i < k; i ++) 
        {
            newccs[i] = (struct centroid *) malloc (sizeof (struct centroid));
            int x = (int) (sumx[i] / count[i]);
            int y = (int) (sumy[i] / count[i]);
            double l = (int) (suml[i] / count[i]);
            double a = (int) (suma[i] / count[i]);
            double b = (int) (sumb[i] / count[i]);
            update_centroid(newccs[i], x, y, l, a, b);
        }

        // Compute residual error E using L-2 norm
        double E = 0.0;
        for (int i = 0; i < k; i ++) 
        {
            double error = 0.0;
            error += (ccs[i]->x - newccs[i]->x)^2;   
            error += (ccs[i]->y - newccs[i]->y)^2;   
            error += (ccs[i]->l - newccs[i]->l)^2;   
            error += (ccs[i]->a - newccs[i]->a)^2;   
            error += (ccs[i]->b - newccs[i]->b)^2;   
            E += error;
        }
        
        // Free all ccs and reassign new to previous
        for (int i = 0; i < k; i ++) free (ccs[i]);
        ccs = newccs;

        // Stop iteration until specified precision is reached
        if (E < threshold) break;
    }
}
