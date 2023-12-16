
## kernel patch

[Andy Shevchenko's review](https://lkml.org/lkml/2023/12/4/1374) of these drivers forced me to use the `device_property_match_property_string()` function that is only available in the 'togreg' branch of the [iio kernel tree](https://git.kernel.org/pub/scm/linux/kernel/git/jic23/iio.git).

the function is not yet present in the 6.7 upstream kernel and should be present only from [6.8 onward](https://lore.kernel.org/all/20231018203755.06cb1118@jic23-huawei/).

for your convenience, you can use the `replace_property.sh` script to tweak your 6.7 kernel to include that function.

