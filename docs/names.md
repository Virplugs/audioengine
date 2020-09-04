# Names

There is a lot of terminology in Audio and DSP programming with multiple words meaning the same thing or one word that can mean multiple. To standardize the naming of the Virplugs API interface, we enforce the use of the following terms in public facing API's and encourage the use of internal/private code.

Name | Definition
------------ | -------------
<a name="sample">Sample</a> | A single measure of amplitude.
<a name="frame">Frame</a> | A set of [samples](#sample), one for each channel at the same moment in time.
