/**
  ******************************************************************************
  * @file    gesture_cnn_data_params.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-08-12T19:12:54+0800
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#include "gesture_cnn_data_params.h"


/**  Activations Section  ****************************************************/
ai_handle g_gesture_cnn_activations_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(NULL),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};




/**  Weights Section  ********************************************************/
AI_ALIGNED(32)
const ai_u64 s_gesture_cnn_weights_array_u64[131] = {
  0xbea15b2cbeb39ca9U, 0x3e717c75bf338f08U, 0x3eba1464beaef098U, 0x3ea35e72bf772b41U,
  0xbe69cd06bea9274bU, 0x3ead3f24be833a44U, 0x3f59b5673f43c12eU, 0xbe9bdcaf3df15585U,
  0x3e8ccf79bfba05d5U, 0x3eaf2fab3e93fb69U, 0xbd0a1f2dbb977a66U, 0xbd931b50bf274be5U,
  0x3e23fc233ddffb14U, 0x3d98f5f5bcc7c24dU, 0x3dcd14543cdba284U, 0x3d9d9afdbd6a6b26U,
  0x3c151288bf49ba59U, 0xbd3dc4683d40cb65U, 0xbc9e4390bc6fb9beU, 0xbe055835be314734U,
  0xbf8c0893bf2dcfe0U, 0xbe1959fcbf066860U, 0x3f2d950a3d3c3f5bU, 0x3eda7d68be8f9c71U,
  0xbf1acb33bef353b8U, 0x3f83987ebf17011eU, 0x3f3be3563d2a2c2cU, 0xbea0a04a3eb6bdbfU,
  0x3f6739ddbee95227U, 0x3e8a09f93dcc8066U, 0xbd87e49a3ea87aabU, 0xbed6d03bbedf5fd5U,
  0xbd81a45b3ec210eaU, 0x3db34f0bbe68946fU, 0x3f1802f33ed47bbfU, 0xbe9ba7023dc297ffU,
  0xbe49d1cbbf0ab8e6U, 0x3e6428963d7c8e32U, 0xbeb73c38bee742dcU, 0xbe8e7317bf184e04U,
  0x3e44a8eb3de1ba66U, 0xbe7c48febc852affU, 0x3cc5c1733e0da1c5U, 0xbc7076f2bdafbe84U,
  0x3dae22783ebda887U, 0x3c4a20483e06108cU, 0x3e7181cbbea0ec5cU, 0x3ee8f317be2653d5U,
  0xbebad89abdccc67fU, 0xbdf53d1fbd9fa70cU, 0x3c27e8a63ea085a4U, 0xbea6c32dbe852309U,
  0xbeb2ae343eb53aaeU, 0xbf0a3a63beb25765U, 0x3efba3603e9dd0feU, 0xbe889f013ec2f0aeU,
  0x3e85c5663d9c01f7U, 0x3eddf8793d8acf95U, 0xbf1afe3ebef7ac74U, 0xbecd17323e0295c6U,
  0x3f20f3bc3e20dd24U, 0xbe55c5143ed809f7U, 0xbd49cc803d815b2cU, 0xbe899c6cbebbbf6bU,
  0xbe30639fbd0a5cfaU, 0xbee744583ea3fd6fU, 0x3ecb244ebc0153c7U, 0x3e0a378e3db2570cU,
  0xbea8b9abbe374f91U, 0xbebd6f8bbee1ddb8U, 0x3f3c4fbb3f028d1aU, 0xbe702640be80bb41U,
  0xbe0de2613ee21c86U, 0xbf0593763ec60357U, 0x3f41c6df3f0be948U, 0xbf0f4db33e27d259U,
  0xbea63556bf3bebacU, 0xbc87747fbf25125eU, 0xbf05b4b7be6c4929U, 0xbf1b4034bf1afdc8U,
  0x3ea70a553e0983edU, 0xbdbe4eb03ebd3827U, 0x3d1506953ec56fd8U, 0xbee72b6f3dad99dbU,
  0xbd5a56adbc9e9a24U, 0xbf0f48313edfbcd1U, 0xbdf41a343dfbe33dU, 0x3e8b7043be882906U,
  0x3d6509dabf0149c0U, 0x3e52b9b0bdf79856U, 0xbeb50d293df47aeaU, 0xbe6d7147bec777f2U,
  0x3ea8e6a63e1a94f2U, 0x3e4a02e0bcfaa9faU, 0xbf0c757ebead677cU, 0x3ee066efbe0e2a6fU,
  0x3e82ce96be38a819U, 0xbf026a6d3ecb8071U, 0x3d2416ba3ec39993U, 0x3ad65ae0be1a0535U,
  0x3f07fceb3ee95485U, 0xbe10c5f53f0bdcf8U, 0xbe80ee073f14f7c4U, 0xbea528253f0b4e49U,
  0x3f1c6b803ecca902U, 0xbebd291c3e80a6f1U, 0xbedb04cdbf0ea9d6U, 0x3de18c78be5ee6efU,
  0xbf28224d3e9fb0ddU, 0xbf2d66ddbf51ea73U, 0x3e7e1e1f3f1b513dU, 0xbd8c20a83f648080U,
  0xbeb849b13e3c9502U, 0xbf0681123f0ebf7eU, 0xbe001f5c3ec4245bU, 0xbf12a8003e4e0e14U,
  0xbea3df663ee51f7aU, 0x3e8bb88cbf1ae1c9U, 0xbea44da43e87a7aeU, 0xbe1f99f03d4dc20dU,
  0x3d280fd13dd72543U, 0xbbb5a99b3c60a6aaU, 0x3e0c3333bcba775bU, 0x3d804837bd1cfe12U,
  0xbe352b983dd4c9a3U, 0xbf47519bbec7932bU, 0xbf8e80c3bf89977bU, 0x3f616651bfa42c3fU,
  0x3f68be6dbf620addU, 0x3f61d58a3eff01d0U, 0xbc1687c4U,
};


ai_handle g_gesture_cnn_weights_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(s_gesture_cnn_weights_array_u64),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};

