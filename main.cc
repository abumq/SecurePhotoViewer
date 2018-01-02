/**
 * This is a quick image viewing tool based on SFML that allows you to view
 * images from secure archive that was encrypted using AES
 *
 * The contents of archive is expected to be in following format
 *     <IV>:<Base-64 of Encrypted Zip File>
 *
 * Full compile command: g++ main.cc external/libzippp.cpp external/mine.cc -I/usr/local/lib -lsfml-graphics  -lsfml-window -lsfml-system -lzip -lz -std=c++11 -O3 -o secure-photo-viewer
 *
 * In order to run program you will need to provide AES key in first 
 * param and archive name in second, e.g,
 *    ./secure-photo-viewer <archive> [<key> = ""] [<initial_index> = 0]
 *
 * Keys:
 *      - Right Arrow: Next photo / Re-position when zoomed
 *      - Left Arrow: Prev photo / Re-position when zoomed
 *      - Up Arrow: Re-position when zoomed
 *      - Down Arrow: Re-position when zoomed
 *      - Equal: Zoom In
 *      - Backspace: Zoom Out
 *      - Backslash (\): Reset Zoom
 *      - F: Enter/exit Fullscreen
 *      - Escape: Exit
 *
 * Author: abumusamq (Majid)
 */

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "external/mine.h"
#include "external/libzippp.h"

/**
 * Download button base64 data
 */
static const std::string kDownloadButton = "iVBORw0KGgoAAAANSUhEUgAAAGQAAABkCAYAAABw4pVUAAAALHRFWHRDcmVhdGlvbiBUaW1lAFR1ZSAxMiBKdW4gMjAxMiAwNzoyNDoyNSArMTAwMJciJZ8AAAAHdElNRQfcBgsVIwK7I3ODAAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAJfRJREFUeNrlXQmwXFWZ/u/tfu/lveS9F0gg2wtZCIGwBHRGrRmXcazSKovBURQVWUSCoFZElBJkahQ1oqWAlOJCYZVgFYUzgRLBcUEdBSJgFSAKBEhCNgJJyL6+vfvO/5/+/9PfOX1vdz/Q0nJu5aa77z333HP+fTvnJVmWER8J1Y5Mv2f6264TXMs7kpz7SYtnmh3xeJq9P4V21WjM8YHzS5q0icefRL/j54vmHrdJCt7j7yeMkGaA8x1sX7mSdj/wQDK6Zw9lVZ63nK5FUlJAcC9Z3E9JXuHvN040eAc1EkQHn2MF45I2R/E5lc8X+Ozmcw+fZehnnPKJ7Uw+74qA1KfP7s5Hk4PTEj4n8fkYwyDhufnxJzzPJE39+DKBBf9Lu7qo75RTsoVXXBGPIRcOrRBCq5cvT2+/9953rRka+veeND2PX0xVflFFYV/m3yn2moSEV6K/j6PSRpuAsgQfVAPseJbdM4mRdd7RR5978jXXHJr+trcRFXBVAkQdIGbvqlXJY5/8ZGnlrl2fHsuyL8u1DgY2I8UBXQAtyJATUa7kIiNynyn9fRyexRVeMTTHI+FQ0Wv7xscd8VYUNsd3d7/rnEsuuXvBpz6VxyX5HLLz5z9PvrZ8+RtfHBu7X5CwtKeH/vXUU2nqaadR97x5+qQCXRFUeDS797d0ZDlCosnYs2q1ZV8iwsYPHqQDTz5JT/z2t7TqwAE6WKkIsH/6H2ed9YEl119/EJ9yzyhC/IVDq1fTr889t+uu3buHhfo/fNxxNOPtb6fO6dMpKZc98DPlAJCb+cffKUKoBUIyBrx/XjhkcNAh5r/uvZc2jozQzI6OSy676qrvDVx0UaDcE7Cy3Jc/nX9+8vX77z+TOePOS5Yupdlnnqnqn4FfKnnOMFEn1/4/ckhThJBykDwviGEYZSy65B2DmzfTD267jTYxUk7o7v6H/1yz5g/2NnmsTJH5d/tDD72Rlfadp0yeTNPf9CaqckfCBZli3XMEcEpTe7gVB/2NHHkiqNnYs6zJrI1YxQAShOgpz0yaOZNOZ7F/87p19MzQ0Ee33HTTxXM/8hHT/4nZ8K6HrbfdlmwbHT1LQP2GJUuoPGVKAwKCl+adORN9RadyY8P5SvuN+nfzw7PF2KnZqbBwYgsRKP0yPGedfjp1i+7Nsos23HprD4KrjMATP4O5Y3k3sxjbzg67qbBaEevKS0qhYftnF1DNKPGv2H/W6jnjOBDtIrYEmWl3Ny1lCfQQK/nVBw8O/AvRGgKR5Y/DGzc6L24aK++uo4+usY5gGVkXzD5v5uJA/8wiqpU7/Vfrvx0dArqWVKfY9ZN7e+n3jJCdY2NL9j344Nqpr3+9G06AEAZ+mvJDXaIzrANpBRZDZtxirG3Ky45K5EK9QqXeNuBt4hN5X6z/JvBsVml0FRODG/ZdhSAGf5c21bExKk2a5HQMX72rOjrqqbgcviWrwvfQCVLKTwAhGcjbwqPZ/WaWjcncVn3HuqtdoE7UqorbF+lLu26wi9o5awvhSaFBESIkqalww2ZV5J1cMU6gOoe4ljKAVkiZKIfgBNqR7ygSbKwv95iIPslriwgpamecYsQUHYYQ80OqIrLkzNRUq4Ll4blDkUFtcEjSLgfFlNQucHSC7mjlE+X0b1Zkrtkb9wW/G9oDYWQRUcW/ITCLoSWnHcrww42yakExNe9ybXGxqWMk5QzadflyRRqKImxjokDGIFzMDlZleJgylstJR0fNVJeIgiEnT9dBv81CIE2pPb5XjQLeZvrGyLF2qoOyel9ucOgYytXUgmRmomFcBnWJISNrhpA2qLzI+cKJGHtnOklRisPbt7twRCoW4YwZ1MFWoVwf3bHDISkV5PT1UYnNS49cQRQAJZegipDTwqpq4HBrX4AocSm8voAxlONufSZGOEQUULlc7wjknkdQCw5pKdNbTRQmJ5OqDA3R0ObNVGazceYZZ1D/a15D5f5+Ny4Zk3DL8Asv0O777qODTz1Fk2bPprSz050kRGbAEWQIciYo4vKAn4sQ4wAjarNSJdqhEXIiiI4rs1gyxx+lyEslcwxzvNhEPc8Q/hNTqlkry0bf6UQUj2VwwwbqmT+fjvnoR6lj6tTaOMQ5VYBIQqhn0SLqWbiQDvzxj/Ti7bdT55FH1rnB3ifza4dgcgCeTQAhDQSH0YH6cSyfzxlCGg7HIcz+XkzYRDCMAhPz1+IJ5lkSL8cKEoQwMkZ37XL2+5xzznEc4m5ZwFPGJlwCaYG+V72Khp5/nnbffz91M6LiqMKEjhjgFCWkIsCjPxKILQv9qPmrOmQz/y8hlMFy3ktT8Cg9EHXCBlB1avx9b6200iHIUXniyu6jiORPEUWDDNyFl15KXXPnOsVNCBQjEvSduP9pb36z45Rx9oodEpFw2vF1mowV55oVcIJHBugU45DURFeSCDu5iccc0uE1vLCcdALiwBDR4BSa0swLQEbXkhasnyBAdQIuDsQcK+Jo8vHH166LLBakFOksfU7EWs+CBbT/8cep1NMTcrdxVBtHrg5pQnyBDoH7BcaB3BwxhKCV1Zcq5TsrSyZtQDVMW4YwDxAT9NxzZbHpKECItKsyh3QddZSznCxQR0VmuT5nltWUE0+kl37xC+rk572+maiua2Ux5iEELToT/zHHRIdxiMyqypPrt2Zexkk+RJBkrn7knScql7M8qqFGJd/KqmmYuLG4RJ5FD6g3nssdEWKdwuevJX4uGx31Fg4CsBDMLRCQC1AwbU1fBDEtQMq4GhUY9CGN9so1l4Nn4FYdG1PEaqZDYmsrhyMaEBBTY0zRbTiGAsiqOn7eZGwRJnHjEOdQCEZMdBXBDYAs6qPSpM4kLymHjiCIcK+LUWRB1U5a94W82eu75NMn3Y3lLFvor1sHIrpsos0mGFtaE4gXJYrQDCbU/sONhOETSzIHM3ubAL3p7SIOMZEk/yFXIMc06TtW6rut7sqbvRaC0OCdWVfGRbEMz3JyJwgcbG+5+nx4QLRUPdtXVONlpibqsGbvr7SoxIrN3IKQSSBp7F4TZzjMhygIK1g1YZ+CGIth2QtATrsDg4442SaWVTMz2TtuMLlMRWq7SDCqtSIDT7WGpNhvip8tGHfLaK8BHscO4mtUr1frCHJBkgAhSVIDodw2anIWjYUZICcSiDGzvBAZIs7yrK5oIi2LINA6maDYatBnQjxxLKtgbIXRXPsZcxBydIwIIwoQYdZbWieEBh3iLtoAq0pREqTzChRFEE7WchH1zj0VtiwRagVkRX7b4XjsO/4OxREYlsk7sogA8hy/Zs9lwJ2IwKw5YYU5dS/3rAMZPHro6kgFUd9mSKE2wu9FoRULhVgbndhEY2X+NQTUC8CMM3ztIiBXD6DIisMlWgbko73h+7xajkMnSfxCX5dl4XYTWzHwDYCxXxIlgvKsjAY9ZMinmoz11hyweku0KDckOi4xm71SVyr1EevYOgLiQ4j5+eTpF+OA2JoyBKjEMbiO6pxN5ti0Qh2Spp1+EKrMfaeYyo3El9Mz9tuMAMw1WAYSEZhHSaShEzMYbMJCXa2snrwj7/2oQ2AuAQFZeCg21+N2OYBvMHFtfuZoKzHYQpYkysnkRnszpagaysq+YjGLgojy3e6jSazIrYmr2PTFnDw4TgEFWjsEKEzWi7F2xBcCVd6lnBaE31H8YoaRKDAAPCfhuDE0gvrB3mFKHNvn6xC52IUIkdmN+uS6IiSNMOgpBHSLAy6YxN6ZRKCZDsBobs6nGQI+32LAiGJCbZWoRkgLqBeJJDZVETk2duQk9PiBUEysxl65+57DIRmOq3aMNih1H+fXQVRHR2ucIH4IAMHLP/M95L5OwmSqpFeRdX3YmS23UKWFuicQkwAoX0Kj7douoCDyOXjTKQ3+jBGXEqNrB+GZplYSICgQrYYk4A4bx5jCOiKrLBZZNVzIqR0HnrVMAqgJS4GSuPDBkKn6JRsZqSlWQQgjyqVUUYEbgNFERS6LUqEtjyLHDakW2wghqVRw0kGBV9WIhRBRwmP2CbG8sEjcNyDAwjb+t+lMogYdYsTuEOZj8QY8oWbRIRqokwCfn7PJZOMUFV8oumTBytj+/TS8dy91TZ3qQuiCqJGdOx1SJL3qIrdmYht8ULQYdbWHivzDqmhg7FiV4tINfEqBxCiPd2j3buqaMsWV1Lo8/vbtVOLxym/JWnrrzExaQ0asxA0BKs6qmjYoIixcIOmjwU6HmJlmgUTLk2hQ0ZSi99plgqbgVQeMHz5MY/v2Uf/SpbTkwgupa2DAJYnkZcNbt9LeBx+kbXff7Z7tOOKImhNqAFIgBhQ/0SAjcFQVARjX3SrgZLyyqHXywoW0+KqrqHv+fJfgEliMvvQS7X/0UTfeYf4u14XTg1CJ6RD7HYmpmEMy0yUwjlBkVau+DMgAbmLKfwolgU7J1CH0fopyzfihQzTMXHDc5ZfTtLe8hcoMcNNFckxevJi6jznGJY8233STA0TntGk+z+G4EjivwatuBzFmPvPnWBNkOuLh8R7asoXmn3MOzXjnO6lz5ky/Ysytp+zrcwiS8W659VY68NRTjtu9rgEuJCBSf4JjKG2GVYdU63NxTBGbKiXzIJ38VDa27GHDiXljKVpTNpXPMRZViz/zGZpx5pnUwYB2Mtjkr3GdLBl+9atp8dVXu9oqEWMCGBEbgQmdEwZpy2O3tsgtBhQRTyyWJRM5yuJJ3r2QuXiAT4cMrFIx3chzmHLSSbSI5zV5wQIaZTGcKUKdfsTch3GD1rdV9X0VFteuIhR0iNGF/IxreDodQLNauHscAW+IiRanxItYRD8MbdtGM08/naa/9a0eCUEhckTFXbNn04JLL6XuOXNonOW3WSpWI+Y9awPyBMSW+RqpIQUtPn6P6IfDrB/mnHUWzXr/+70+o9g8N6TwdSGwYxhxokuqg4P1SLK+L4uUuUeMtNPvWJuFw01h3EISZaM+CcGPK2KqxiEYrYzCycZRQuFj/DmbJ+eqBlF85IWwlYqnnHACHfvpT1MXU6dQrFFcVlu1Wk8stZO3h/5jH8pbP9y/GByjrOPmf+ADNOe889xCGp8ejrx0/17NQPaedhpNe8MbaIg5K4OxovWFCt0+K2ryGpwjIg04xPWSaeBrSBWqdFCRtdYqAwOOsJcqRac8WEHIkUuWUCeLIPMlYpu+/vbUiTJnAvOzk+bNowXLl1M3c8woT1RElzcoolToRIoUnMhAi0i4n5Gx78UXadY73kEDH/wgpbJeA3RjnuNpSBGECfKmvu51tF/MebWc0CP37zZCFXElukr0megQWVkQzqGmMuGHnFOlclHWUq/iAT/B7LiHOxtB7xTjNMg5RF6HCLX7uilg9YZJxjKaT5PRTqcwp0g9liumLgqxNDvAp6nop4x57MABGt6xgxZfcAHNvegih4yGSHUBJ9o85BQl36X+ls/9IFIgqiHfBvm6rL59hK25jTynlLwO8S8yP6Rm9ibJTtud4SAP/ACfuxmzk5lqZjIlH8lAnsafnSKKxOHD8IZZYiqDE5zcRA5+Ria64OMfpw033OAUZ6eYxIbgl+EcmnlZYWBI9clBNrkXnH02zebTVbJYjEzf4WNmAOTcoYqfZSa/AF11rBdffIwqUW/j9+7hawLPMSUQsd7G/QvzliPUNosJWEkeHOEXvMgdbuVTVo/2sSKcxYjpZ6RIiU2Jv5eU3eX7wWeecVQjbO1DFBCWCRJXaPmYmcrvE05Z/LnP0frrrnMVi26yFnqxPlpxB1SbOJ3I4lSU+KIPfYjmnH8+JeLggTwP4mpgMluxBSJaECB1xk70CFJFNDHVj7BkGRJi5t/7FAEiYaSdEEVJ4TusxFwOiSsLHcMsG7Z4ltyoEmHMvvYwn4f4RZu5XSd/72Sqm8yDO4IHMkn0AVPZ7jVrnOMnBc9+YrZ4NC4VBQA6u9/ex/cnsZ9yzLJltP766+nQpk3UY0pzgocgc4if3ckO3cmsvGe97301MaVFcxbf8gFTTA1gZBfiWaLIpWZYACyKfZjviW+xl991WHXFuHKZmK7iu0kOpKJwFaTI/UoEB7SyZGDjPlOimPS/9UwVywY46VAoYTMjZj3LxjVstj7DlLJuxQoaYaXpA5agQ7zXjzkVUP5eTjOC+tiaWfK1r1G3rP8QWR2FVAqRkNWTRtK/UOlS1hdzP/xhKk2Z4nVWUKxtY7FAal7KQMe26957adWdd9IWmTfP9yVBkFTnqwFRUuq3zXlSRYLDM3BdPIO06W99MKEwDG8v8OJH78vLJdLVywNe9dhjtOX7368BBS2QqN+W1M3npIEBWnTlldR7/PENdWJFB8p/seL+ifXFTHZSLajZagGOiUcLerogqSJlhLn/ka98hfYzArr4upMUfF2kSod+t/lh8DAm7rwjjvbmbRbmgW9YNjlov+WeQ4YOZlwp5H9WrqR3sBgbYJndyRTu9YXZ3/nOUR0o5nDxpMXE7H/ta33oxUeZC5YYYEJoytKl1MunC4xOJB5mFAwcuYfF1BNsBW5mbp0hyFWDIVXAWzTcaqSTiPhQBbSDkI68RsgFJUMMnIg0OSfVto2gWTzghxkpr2N2XsgU7oBnnjDmOwoOH1k2keZvJC3zIZb/TzRPg7sYJe0gRC0l5EgRwY9fcYXsUUIzRF8q0N3eWAZo40qy8HkSvM8IOy4isi8xRBo4xGI6xgmeQ6imS0ogJ629HMLGYoUJx/zyjjtow7XX0tiuXY0LfJohxHSJyXhwMNuJZdnYsW3etYKHg/rg/Y88Qr9/73tpHeuM2UxospFbB0gKO2xjN5Me+BnDCN+GCEFuakqyyHq2fLpDucYreqAGAf1UnsxsNgv/8KMfOZ3iS1RbWUuR09gQfm8VQoH72USf9dCpmc2yZnE1i6m1zBmzmTO6IoPHIwLOeB1hu9G3OIV7hE3AZ6wAa0mE6ZKemSIFKU/+lxBBByOknyc2xNd/z5wigDiGrZ2yrPsDrzaoGY6pKAZeO+ET5IpYREW+DxonHoFUC3sIZzz72c/Sc2w9zhNTWe9JKajf/RMi0x4BqltMryRR/3EK2x7FRZ/yecjuxPSLFgOKKb8vKyDLuKXDZD0jZArrDrn+JzYVZQALLruM5VqnL0G10EMDJeXpmYlGAPLaQ4LKFLf5SuZvCGc8w8h4lkXtXOZyIa4xvicmtDl0JX3W9lRMLSVNoY6Ny4qC+oX6UY5XUHXnIiHnN4qpFMSWKX3TM2VVrBJ4nMzXhGseZEUv1+ZeeCF1TJ8eWF9+yZxFd18uElogJMu7B8HDA48/Tk+zIbJ2926aKalbyedIGyioq5rFaABWoHvdISf8jkeRNP6sxGVAk3LnQ3WZiD4IEQXKHs1ihyANpWDIvJ8pTaywNcIpfMy/9FLnNbt9pKIcxCtAQWscAVK87lNuHWZras3nP08v7NnjFHiXcHe5DqpUV0BZwNJZVEniHb4GuIGIyqhQn7jLGMtyqe68gRPVfRBzEA0RJapzDCJDgJuahQTBOpnYZEaSRIafUJ0yd9kyKuua80AZ/wWRYmLKh/appjMOPvkkrfnCF2gzi6tZ3d21ULukFbQcyrz6Esa7tC8ze6sKD+efKMJiDvG7U0TDijef8X5IEJePLAZ7AYos9FFMuVv2zAcWDUkyoZ4eOorbrGdOkT6O+djHajkJW5SZZYVO358HI/WaKZPtojPWrlhBW5lD5jCBuOiv5k8McVVcyqDUbxziDBzlEtxS2xBQkCXEI4kdw4PYgVGSYbccAR11hdchmP8wmarICDaFkSXLfX3Uy5S3ljlFJut0ylFH1QGWTWBxzss4HBUqVx5++ml69uqraduWLTRDKkpkoagV14kOtKyoAV3nWMIKRqoTb8lgB4jI08fxgdFeOcbyBo0IioOOHvvQNrWsm7RVNnclPrB9kkdKby8dwYja9OMfu4mKThHucYj8S+5omiS+PlliU2uvuYZeZGTMZHPcIYNFqsyjalX9UvLD10oW4FTESBbQLWGmur6I4RemZZseGRbKKZ7rFkSeMvemLTV67anqCC+aFAGOM5Q77C2Y2CpPnky9jLDn777bhe8HLrjAlQ25WFa8l0ocvi+yvvLSvVFOQ8TRoTVraN2Xv0z7Nm2iOQMDvjDQqkJSW7KghQ8yT19JktWqMOV7SVPcVrPrdQjVDR+DZ0FwNCFqLCUtN2BR/QgTRwZ4X6+kyCipaeuUuSKBMIxt9rmFsE0ZKqfIthd9/My2n/zEAWTe8uU1nWLUrKID63NbmsIYyMQlCNrPyLZt9NxXv0p7NmygqYIMTTRltksEFr3Zu6y2TMWVKwgRGFiam2p7vmOUPFW/JW01Xgo99ZpPR5Fsi+RfHPlNAEHeUxeAmdjSfIPJXEOQj1NZiaqcU6ZQB1PflnvucfekRkpKbmxLPN93k1JMisbufRlz+DTVOrh2La2/9lrar8hw+6Cog5paOjaqZvfIobr3n6hodRJFU9foh6CXjqfpm7wd5dBEHsnjI/tEcUXwmyLFhckfjwATXyZytOIEnUHn3ff3Uy9zxraf/cy1mSfWl6SCLU+PVN8k4husJYkSTaPbtztk7GVx1SucMWlS/b4RDYRPPJdbhDpJfHlSkPNQqYHL1YJIMwC6GYegDgkiJg0euukJDZ04UQXKO1VLynGHloSaGeueB3FFxjkW3lZRJMVnAqAeRtZ2QQq3GTjvPGd9mSz3vk0bIisuih5ct442fP3rdIA/+xcupJLm/a2YzZcu4Z4oJu5AB1qexqzI2u2SE3kC1HHLk6jIirOvRUiJ/ZBAg6JXaRxgrGgISTFPYaIq0h1BehYQYZTuzVs0BFiEdPPnjl/+0nUtnCKlp+jINfNT4siz44wdO2pi6tlnqe+442o6wzxp2NLQK16LYNscrS4gqmw0wGFUuWzXUZ9Qo+iKC//iHeW68yaGOiOwq/ETfA8vgsD89cAGbxctJsdlZokpB7gKc+aUnb/+tQOCs75YpLXtMNpE+dnDzBEbb7hBdu+m/iVLasgwwKoTGtSS2aowTWgF1ZPK1YlxDRCVcW8cQrHxNItPk1pZjsOMiOJW3vMG5JjJm8YcEAE+UOLGFbBbqDeFzcTUBTGmLxK1vmQv+l0PPODuS2GbT8W2ElmmM3btoo3f+AYdeOYZ6hVkdHbW6MC4zJQr7EpnTmBi60qkL0GYirV4AwI8DTZIxGnS0kv3HIJ6o9vEFHqLeVaCKSrTHVgtYlQWcwpaXl50mY7BawCYxEpN+drO3/zGjWvO2We7KDEW6vlqQUCwAHVo/Xra+M1v0mG2pgQZZXU6CcSkmd6+IFtN8lT78ECHisTE3oWIgK0/0gL4GSHE60IIrCzkmZEYiz5OhSFlwzoiA3RHoD8Q8Gjqxus+wLJJ4Let3upQUSW1UJJ1nM9+CkkkWR03lP8m8sb27KGNN97oCvf6TjqpJqbksLWKyo2xA2mhEds0wUIs3nw2f0rH6ihakKEhFlITONUkFvokBvmiQotYqY/EYiC2mwPxBQD1esJEWGTyIiVS/Jz1DT4Kflo7t6BHxNd997n+Bs49t+an4GYGarENMmds+va36RDrjr4TT3QbLHsH01Z6mZIG8Yc6AyOy3jLSTXjsncalWUyIAKs8aVN0WHGEIebkuEEcv2+FjHjFU1BsDb8DzogsMkKkGmL1t+gUWXK279FHaesdd9QWZVq/quxljcnmm292Crx38WJXGIfi0IvHHG6OK+ATDQUF1fs4Tyy6UMSi5YmRb1xDUnSYlWXR4g221TjmJAwRhkEDagocEOiPHGohZXcUW75fFGUKWF+cFsWkJO4lvoMEIPc+9JCjcikNFe6RdkObNtGmb33L1QMLZ4iYskLoBmtIxmCWk+3j6Ckxa7CkbPy2IYKttURjBkVenndu2UW30pnIL5Owt2Ipaa6nHiPDh0vQekKKM5HkH04C6k+KvhsSDSkgo4OklcXWGCHdc+fSnocfpq0//KHLU0gx9abvfpcOspiavGhRbYGpWEbx2hTzMdDQyLMW43nAZ0Bk0Ri9oxxZVj5HlM8ciXEISqL2NxSxSZlHjjVUgJxYmaOYiDkK1yAGu2arF+/708mJnzKFvfp9jz1Gleuvd4t8hrdtc39vsWThENMRumI2sHbQx8C0MVB9UP0OEYUGSw0Cp2icBCK+eYGeT+GiJ1/GGAqaaZTzHTEfU0nubxsk6pacdqiLMKhoIg29ewH85GOPpUEWVSJy5LuINfx7vYZEZ+XYEjkoxvYlSGn9Lwvl7SfpCzBiAKjocs+oQxkjoNVfszM8NG6tIfItSfyumW4BKNRpoY8SW1SBCIA8hqec2EvPQQqGEtCpRKMhwfSumOSsU3pkt2uz5gx4IKr8pjlG1RDmyNRMDZBhXAnp50yBn2gI3xMo5Ga8xRV56973gN+VxoxoEqwP4c5OoOgh7ARRGcjOiIriMHsgnoq4IrbOELAIYEAUvrshnALARkpt2BgHivQ8QMGICPYFQzGGiTJbRxKZ8OjDoUJH+AIBNjqGfGWd9yCTJN/c5dMivLHtjpyGAUdU3kWfXr+gQ2mAztErDUiNDw1t2BYdQYILs41JfZWU5Uq8aLLiBiNAs7TibZgMZvH7STe5hDZmXRUdghDzRYKjqiFjrO4Lpm6AiAYSbB2L3ACKzorJAsrPE03xvUiUBSZ1Az6yoB4qQco2BKkY8mLJdqUo2NOqQdfEKWHjANwnSxHg/mK0VjhWYf4RzfslbTV4V6vdQbxFO6jEmI2ow4sGQEazmiq0rij+jghEbx+5ATm0ACEeyJZaNS7OqWYJRJjpEHXuvCcOBFaNiRA5B66ZuK9EiPGIUxjrc0GhXO1CkpyBgUNpJuvixvllUqk3lqZ+nXVJ/lpBktT3MdRNZxJbV2FyEzNuqvD8rm5A4X5fdgBwBu1wtyG/WAcMh7w94wPPGIgGi7yDjWFgRwq/NYbtUaLjwb0T/Rp9S27BjhfGEbLidhzOalZf4JN3BGVAPMGV/OMHpkPk4XFFyqh2KC9ItcqiQ6r5IoeoqlQZfGIkVSmcbIkYeMqJiQMEtnGcmbxozcD9BhFjQMTdJnAnOCuQw/2sANBW7BDsxoAIgNPvdKGfEs4RohW4jQHs5BxVGHYozFzko5aHCcxeQ8q4/TCWQuwatiVb6JatSURTo7ENaz5A3gc2OFI95KgNeEH+RJBmvzH/gQHKWHzGCDGg67uDrflwuxDYGsP/BRzcq8TaRgiq5mzMU1XuGFeCDjgEYKsR4AVqwHgry8uIzunT/R1XBMYTFizL2o7DCnxL4hvLyWL8sogvtZQsd2EZuEyXHIhv4/66gW555IocIAdiwLYNAlysTNrCsrY6riNln3eAWPKIyWBpc/TXEqqAAGlXxTJTAzZuDCAFc7YLkO70I9xge5nIOnRZtjAo9cJS/8XnqIosjJ73l0pjJf3zTQ0c0n/qqaNjq1bdzHbzxeapj6iIOmjpTMO4IJCvd2l4QGztDgZSWf5kndrerlBZcxbmSKUYaokQgkFHhxDb0ilGSJtWlucaqN9FhGCZTxV3OiLyW/s16AuznhghVUDAmH6OKncIQmS/mP2KjEPymz/HlJhLKmXmdXVV5W9l2RHkQxZeeWXS/Z3vOLbqUEU+YuaavISvyeJ4WT8oOzrIZ49aPLbezlc0EgWrdc1ts78CZ+vwEvjEw9aUpGhRFTlsNgH4xHoqFKW4bWvwXXdawAiFHBb/rUTixqynihLoiOkKMYL42pBuMDOoiBhUkYWmMCNq2QlnnLEdht74Z/POnzXr8lu2bbs4BaTIw8O6PUVVFdJgWqtm9JWLSb0qJQHE+MI6VcKIGGtPet/Ahs9gpQZB23YOQ07siPkUA3yPzVJDiEWzMGxuv81iMrO2ohxiylvE/Yhyju5p4isYpe30jo7Rme95T+C4N2ykfPIXv3j4lE984h+fHRp6tFPN1AoMTtjPlQCJpZWDCKxlxSUNBJFPgs8SiqII8Aj8ACHWvgVi0F+oNrmHCInvewRgPC+rF+h5favPGHJsG42Kzl1iz2IhlusS4q6Pv/vd/y3r5wMiyur7vXuO33zjjcmK6677Hg9mWaciBFnNgF0CIONKqiAxAwPXgQQADvIFLYDc0LZJewMU/o7vE7SJ8xQVuI79ZTn9BVwGos6u29I++z1cqdz1sfnzL//nVas2Rd0lCQwaxbBbV37LzTf/2wujo/cgAIgaRQau0o0ziw0U79+cv/g/CPlHpmxR2yKE4BF710Y8AQLx+ShcHgM8y3kOuYhAvNm4RXSxEXTnBwcGVrz+d797Mme4iXFIXkA3275yZfK/X/rSkT/du/er3GBZGZW3AtRvKg8ASiIxhsCL10oUAbUos9ae9mjsP68vHF+WI8Ly+qtGQA8QgpDVtuN1/fKrga6uXyw7++wbF69YMV7QfS6HxN9p969+lTx/yy303OrV07aOjfXyjV5+wWn8+XBSq+V6O7/8R9x0HiNqN3++iq/18Lmavw9ltaVyh7jtdAbCVP5+RFa7vo6/L6DaQqFtfO8YBtAakj6z7DKqLdNez+cMbvuiDkeK+dYyIYjxLpWWYids5nOA6otWn6bafSxrkvtDae1dztvQ98ozR/K5Na0vCz+qUvvbtDLWsaT2HtG3h/l7lz63kzt4Jz+zQaEp4zuW7+/laweqtT+lumNaubxjUX//rnnnn3947sUXtyQiQYgtAMojxKxlD7VDdrs8HD1LTfqIubKIS6nF9Wb9xN+pSX9Zi/7jsbS6Xgjvguv+GXMREmqUPEWATHKuHS56QcEgsoJ7MeCKJEdW0E8zKUhN+msGKEtPNA1gN+m/aOz5HcXblOpZlFKfCOc0o+R2j4lQebN7Wc5n0XtazTN+VzOCaxc+/ohLUFs9jB20ap/lPNPOUQTcZnq+JeUVAK+I6LIW/Vg7fD7PIi6aV958asHuJi+Mj24qlqFFfVibhFqze548bnW0a3S1K4aLOCVuWwSjVhzVkvj/D1gmIBWLmz72AAAAAElFTkSuQmCC";

/**
 * Path where images are saved
 */
static const std::string kSavePath = "/Users/mkhan/Downloads/";

/**
 * Download button default color (at normal state)
 */
static const sf::Color kDownloadButtonDefaultColor(255, 255, 255, 75);

/**
 * Download button hover color
 */
static const sf::Color kDownloadButtonHoverColor(255, 255, 255, 200);

/**
 * Represents the move factor for positioning when zoomed in
 */
static const float kMoveFactor = 20.0f;

/**
 * Right angle (90')
 */
static const float kRightAngle = 90.0f;

/**
 * Represents single item with it's attributes
 */
struct Item
{
    /**
     * Raw data (this may take more memory)
     */
    char* data;
    
    /**
     * Image size (file size in bytes)
     */
    std::size_t size;
    
    /**
     * Filename in archive
     */
    std::string name;
    
    /**
     * Image object
     */
    sf::Image image;
    
    Item(char* data_, std::size_t size_, const std::string& name_)
        : data(data_), size(size_), name(name_)
    {
        image.loadFromMemory(data_, size_);
    }
    
    Item(const Item& item)
        : data(item.data), size(item.size), name(item.name)
    {
        image.loadFromMemory(item.data, item.size);
    }
};

struct Viewer
{
    /**
     * Viewer's texture object
     */
    sf::Texture texture;
    
    /**
     * Viewer's sprite object
     */
    sf::Sprite sprite;
    
    /**
     * Viewer's global rotation tracking variable
     */
    float currentRotation;
    
    /**
     * Name of current archive being viewed
     */
    std::string archiveName;
    
    /**
     * Current photo index (zero-based)
     */
    int currentIndex;
    
    /**
     * Items in archive being viewed
     */
    std::vector<Item> list;
};

/**
 * Global viewer object
 */
Viewer viewer;

/**
 * Returns true if subject ends with str
 */
bool endsWith(const std::string& subject, const std::string& str)
{
    if (str.size() > subject.size())
    {
        return false;
    }
    return std::equal(subject.begin() + subject.size() - str.size(), subject.end(), str.begin());
}

/**
 * Unpacks the encrypted archive and returns temporary filename of unencrypted archive
 */
std::string unpack(const std::string& archiveFilename, const std::string& key)
{
    
    std::cout << "Unpacking..." << std::endl;
    
    std::ifstream ifs(archiveFilename.data());
    std::string archiveContents((std::istreambuf_iterator<char>(ifs)),
                                (std::istreambuf_iterator<char>()));
    
    if (archiveContents.find_first_of(":") != 32)
    {
        throw "Invalid encrypted data. Expected <IV>:<B64>";
    }
    
    std::string iv(archiveContents.substr(0, 32));
    std::string contents(archiveContents.substr(33));
    
    mine::AES aesManager;
    aesManager.setKey(key);
    
    std::string zip(aesManager.decr(contents, iv, mine::MineCommon::Encoding::Base64, mine::MineCommon::Encoding::Raw));
    
    std::string tempFilename("/tmp/insecure-archive-" + mine::AES::generateRandomKey(128) + ".zip");
    
    std::cout << "Flushing from memory..." << std::endl;
    
    std::ofstream ofs(tempFilename, std::ios::binary);
    ofs.write(zip.data(), zip.size());
    ofs.flush();
    ofs.close();
    return tempFilename;
}

/**
 * Loads the items from insecure (unencrypted) archive and returns the list
 */
std::vector<Item> createList(const std::string& insecureArchive, bool doCleanUp)
{
    std::vector<Item> list;
    std::cout << "Loading..." << std::endl;
    
    libzippp::ZipArchive zf(insecureArchive);
    zf.open(libzippp::ZipArchive::READ_ONLY);
    
    const std::vector<libzippp::ZipEntry> entries = zf.getEntries();
    list.reserve(entries.size());
    for (auto& entry : entries)
    {
        if ((endsWith(entry.getName(), ".jpg")
             || endsWith(entry.getName(), ".png")
             || endsWith(entry.getName(), ".jpeg")
             || endsWith(entry.getName(), ".gif")
             || endsWith(entry.getName(), ".svg"))
            && (entry.getName().size() > 9
                && entry.getName().substr(0, 9) != "__MACOSX/")
            )
        {
            list.emplace_back(static_cast<char*>(entry.readAsBinary()),
                              entry.getSize(),
                              entry.getName());
        }
    }
    zf.close();
    list.shrink_to_fit();
    std::cout << list.size() << " images" << std::endl;
    if (doCleanUp)
    {
        std::cout << "Clean up..." << std::endl;
        remove(insecureArchive.c_str());
    }
    return list;
}

/**
 * Reset zoom level to original, the position to 0,0 and rotation
 */
void reset()
{
    viewer.sprite.setPosition(0.0f, 0.0f);
    viewer.sprite.setScale(1.0f, 1.0f);
    if (viewer.currentRotation != 0.0f)
    {
        viewer.sprite.rotate(-viewer.currentRotation);
        viewer.currentRotation = 0.0f;
    }
}

/**
 * Add scale level for sprite
 */
void zoomIn()
{
    viewer.sprite.setScale(viewer.sprite.getScale().x + 0.5f, viewer.sprite.getScale().y + 0.5f);
}

/**
 * Subtract scale level for sprite
 */
void zoomOut()
{
    if (viewer.sprite.getScale().x > 0.5f)
    {
        viewer.sprite.setScale(viewer.sprite.getScale().x - 0.5f, viewer.sprite.getScale().y - 0.5f);
    }
}

/**
 * Moves vertically if zoomed in
 */
bool moveVerticallyIfZoomed(float moveFactor)
{
    if (viewer.sprite.getScale().y != 1.0f)
    {
        viewer.sprite.move(0.0f, moveFactor);
        return true;
    }
    return false;
}

/**
 * Moves horizontally if zoomed in. If it's moved returns true otherwise false
 */
bool moveHorizontallyIfZoomed(float moveFactor)
{
    if (viewer.sprite.getScale().x != 1.0f)
    {
        viewer.sprite.move(moveFactor, 0.0f);
        return true;
    }
    return false;
}

/**
 * Get window title based on idx
 */
std::string getWindowTitle()
{
    return std::to_string(viewer.currentIndex + 1) + " / " + std::to_string(viewer.list.size()) + " - " + "Secure Photo - " + viewer.archiveName;
}

/**
 * Navigates to current index
 */
void navigate()
{
    Item item = viewer.list.at(viewer.currentIndex);
    
    viewer.texture.loadFromImage(item.image);
    viewer.sprite.setTextureRect(sf::IntRect(0, 0, (int) item.image.getSize().x, (int) item.image.getSize().y));
    
    viewer.sprite.setScale(1.0f, 1.0f);
    viewer.sprite.setPosition(0.0f, 0.0f);
    std::cout << "Opening [" << (viewer.currentIndex + 1) << " / "
                << viewer.list.size() << "] " << item.name << " ("
                << item.size << " bytes)";
    std::cout << " (" << item.image.getSize().x << " x " << item.image.getSize().y << ")" << std::endl;
    reset();
}

void next(sf::Window* window)
{
    if (++viewer.currentIndex > static_cast<int>(viewer.list.size() - 1))
    {
        viewer.currentIndex = 0;
    }
    navigate();
    window->setTitle(getWindowTitle());
}

void prev(sf::Window* window)
{
    if (--viewer.currentIndex < 0)
    {
        viewer.currentIndex = static_cast<int>(viewer.list.size() - 1);
    }
    navigate();
    window->setTitle(getWindowTitle());
}

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <archive> [<key> = \"\"] [<initial_index> = 0]" << std::endl;
        return 1;
    }
    
    bool isFullscreen = false;
    
    viewer.archiveName = argv[1];
    viewer.currentRotation = 0.0f;
    viewer.currentIndex = 0;
    
    std::string tempFilename;
    
    if (argc > 2)
    {
        try
        {
            tempFilename = unpack(viewer.archiveName, argv[2]);
        }
        catch (const char* e)
        {
            std::cerr << e << std::endl;
            return 1;
        }
    }
    else
    {
        tempFilename = viewer.archiveName; // insecure archive
    }
    
    viewer.list = createList(tempFilename, argc > 2);
    
    const sf::VideoMode winMode = sf::VideoMode::getFullscreenModes().at(0);
    sf::Image winIcon;
    winIcon.loadFromFile("icon.png");
    sf::RenderWindow window(winMode, "Secure Photo [Loading...]");
    window.setIcon(256, 256, winIcon.getPixelsPtr());
    
    if (argc > 3)
    {
        viewer.currentIndex = std::max(std::min(atoi(argv[3]) - 1, static_cast<int>(viewer.list.size() - 1)), 0);
    }
    
    viewer.sprite.setTexture(viewer.texture);
    
    navigate();
    window.setTitle(getWindowTitle());
    
    sf::Texture downloadTexture;
    sf::Sprite buttonsSprite(downloadTexture);
    buttonsSprite.setTextureRect(sf::IntRect(0, 0, 100, 100));
    buttonsSprite.setColor(kDownloadButtonDefaultColor);
    const std::string rawDownloadButton = mine::Base64::decode(kDownloadButton);
    downloadTexture.loadFromMemory((void*) rawDownloadButton.data(), rawDownloadButton.size());
    buttonsSprite.setPosition(0, 0);
    buttonsSprite.setTextureRect(sf::IntRect(0, 0, 100, 100));
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            bool newPhoto = false;
            sf::Vector2i pos = sf::Mouse::getPosition(window);
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::MouseMoved:
                {
                    if (buttonsSprite.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                    {
                        buttonsSprite.setColor(kDownloadButtonHoverColor);
                    }
                    else
                    {
                        buttonsSprite.setColor(kDownloadButtonDefaultColor);
                    }
                    break;
                }
                case sf::Event::MouseButtonPressed:
                {
                    switch (event.mouseButton.button)
                    {
                        case sf::Mouse::Button::Left:
                            if (buttonsSprite.getGlobalBounds().contains(pos.x, pos.y))
                            {
                                const Item item = viewer.list.at(viewer.currentIndex);
                                const std::string extension = item.name.substr(item.name.find_last_of("."));
                                const std::string filename = kSavePath + "secure-photo-" + mine::AES::generateRandomKey(128) + extension;
                                std::cout << "Saving... [" << filename << "]" << std::endl;
                                
                                // item.image.saveToFile(savePath) causes problem because of version of libjpeg in local dev
                                // so we manually save the raw data
                                std::ofstream ofs(filename, std::ios::binary);
                                ofs.write(item.data, item.size);
                                ofs.flush();
                                ofs.close();
                            }
                            else
                            {
                                next(&window);
                            }
                            break;
                        case sf::Mouse::Button::Right:
                            prev(&window);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case sf::Event::KeyPressed:
                {
                    switch (event.key.code)
                    {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;
                        case sf::Keyboard::F:
                            if (isFullscreen)
                            {
                                window.create(winMode, getWindowTitle());
                                isFullscreen = false;
                            }
                            else
                            {
                                window.create(winMode,
                                              getWindowTitle(),
                                              sf::Style::Default | sf::Style::Fullscreen);
                                isFullscreen = true;
                            }
                            break;
                        case sf::Keyboard::Right:
                            if (!moveHorizontallyIfZoomed(-kMoveFactor))
                            {
                                next(&window);
                            }
                            break;
                        case sf::Keyboard::Left:
                            if (!moveHorizontallyIfZoomed(kMoveFactor))
                            {
                                prev(&window);
                            }
                            break;
                        case sf::Keyboard::Up:
                            if (!moveVerticallyIfZoomed(kMoveFactor))
                            {
                                viewer.currentRotation += kRightAngle;
                                viewer.sprite.rotate(kRightAngle);
                            }
                            break;
                        case sf::Keyboard::Down:
                            if (!moveVerticallyIfZoomed(-kMoveFactor))
                            {
                                viewer.currentRotation -= kRightAngle;
                                viewer.sprite.rotate(-kRightAngle);
                            }
                            break;
                        case sf::Keyboard::Equal:
                            zoomIn();
                            break;
                        case sf::Keyboard::BackSpace:
                            zoomOut();
                            break;
                        case sf::Keyboard::BackSlash:
                            reset();
                            break;
                        default:
                            break;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        
        window.clear(sf::Color::Black);
        window.draw(viewer.sprite);
        window.draw(buttonsSprite);
        window.display();
    }
    return 0;
}

