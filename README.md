UE5-MOBA MULTIPLAYER ONLINE 

 # 1, Basic Character Movement and Animation
 
 - 1, Set up Base Class
     1. Create basic classes: gamemode, playercharacter, playercontroller
     2. Assign the new gamemode to the project and modify the gamemode.

- 2, Set up the Camera and spring component
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/bb79df51a122d6f3992b6581913d85003c9085f0)
 
- 3, Add the jump Input Action
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/3587d6761cc81668069bce302bfd820d1f5ba688)
     2. Assign the input action and input mapping to the bp_playercharacter.

- 4, Add the look Input Action
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/4bd94ba7b4fdfbb07e7d70b58e38ac3c6b9945f0)
     2. Assign the IA_Look in BP_Character.
  
- 5, Add the Move Input Action
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/74f5d7e9db7afa5d82c2f775f2cb485873664e46)
     2. <img width="1055" height="678" alt="Screenshot 2025-09-10 192605" src="https://github.com/user-attachments/assets/68149941-20c7-4a3b-a05a-5eddc639abf3" />

- 6, Create the Animation Instance
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/e44113499653c8782cea020a6fa410d1bd222252)
     2. Create a new Animation Instance.
     3. <img width="532" height="74" alt="Screenshot 2025-09-11 201706" src="https://github.com/user-attachments/assets/f9d2031f-b9c2-4f64-843f-321f2e89c7da" />

- 7, Add Speed Attribute to Drive Ground Locomotion
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/00f1589532965f06edd250c280acdd0421c2349a)
     2. Set an automatic rule. <img width="525" height="34" alt="Screenshot 2025-09-12 202156" src="https://github.com/user-attachments/assets/dc63bca5-9814-472b-875c-341a772a87f3" />
     3. Using Group to smooth animations. <img width="519" height="108" alt="Screenshot 2025-09-12 202319" src="https://github.com/user-attachments/assets/3cbf344d-fcb1-43e6-a7bf-b229bc915385" />

- 8, Add Jog Stop And Jog Lean
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/5ddf44e7ab32cd4cc8aaeaab303af7a15f404342)
     2. Add new jog stop state, and new BS_JogLean.

- 9, Add Jumping Animations
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/8daf980f3738ffc9fabb30b265aba34e789c7694)
     2. Add a new state machine about jumping (jump start -> jump fall)-> jemp recovery -> on ground animations.

- 10, Add Look Offset
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/2d75d24f79b3ccec390462816fb4c0048731f98c)
     2. Create the new Look Offset and Add it into BS.
        <img width="704" height="333" alt="Screenshot 2025-09-18 201727" src="https://github.com/user-attachments/assets/d61eb977-6b82-4cc2-a898-3973a556bb03" />
        
- 11, Add Foot Rig Control Do Foot Tracing
     1. Enable the Foot Trace.(using rig to track two foots to grounds location.)
     2. <img width="1081" height="505" alt="Screenshot 2025-09-19 203949" src="https://github.com/user-attachments/assets/a3b52f90-f44d-494f-b8a3-145e2a913879" />
     3. <img width="1043" height="267" alt="Screenshot 2025-09-19 203935" src="https://github.com/user-attachments/assets/5336acad-f64a-4d36-b0a7-28cfcdcf44b8" />

- 12, Smooth foot trace location and Modify transform
     1. <img width="1302" height="672" alt="Screenshot 2025-09-20 175734" src="https://github.com/user-attachments/assets/dbf41731-486f-429b-8571-d5ec3c9cd90e" />

- 13, Add Foot IK
     1. fill up add foot IK and use a boolean to detect when is not on ground, the Zoffset set to 0.
     <img width="758" height="585" alt="Screenshot 2025-09-21 185657" src="https://github.com/user-attachments/assets/ccdc7a31-0995-4b3f-b7bd-5e37a8b56d88" />\
     2. In the BS, use is on ground to detect the boolean.
     <img width="684" height="220" alt="Screenshot 2025-09-21 185649" src="https://github.com/user-attachments/assets/881469a6-ade2-4954-8cab-503664add46b" />

- 14, Add Abilitysystemcomponent
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/a7526eb9afeed6dcda7873a3ddde3e0d28a2f71d)
     2. remember that override for getabilitysystemcomponent().
     3. Fix code issue: [code view](https://github.com/Lazy22zz/UE5_Moba/commit/d39f21f08decf354041bfc6c5b565e7105311988)

- 15, Add Attribute Health, Mana
    1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/fc4fa8e32c6e9ecc63221471365f5541f3851ec0)

- 16, Apply Gameplay Effect to GameplayAbilitysystemcomponent
    1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/fe04cd74ac788884822e5c618ef21c37c5e32bbb)

- 17, Setup Server Client Call Chain
    1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/9cf5e3483108c9308280b5ff84cf4e953d68f9a5)

- 18, Apply GameplayEffect And Replicate the Attributes.
    1. Apply Gameplaye Effect.<img width="757" height="384" alt="Screenshot 2025-09-28 190351" src="https://github.com/user-attachments/assets/b48ed116-9ee0-4ee0-b0d9-7a67dfa2e0ce" />
    2. Repliacte the Attribute in Client.[code view](https://github.com/Lazy22zz/UE5_Moba/commit/24fa45186304c5be0488dea0d312c6e6b745812a)

- 19, Add the Value Gauge Widget Class
    1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/6b7f5e889d7fad0690ec39a666b1659673e36fdd)

- 20, Add the Value Gauge Widget Blueprint
    1. Check the Program's `/All/Game/Widgets/Gameplays/ValueGuage/ValueGuage_WBP`.

- 21, Add Gameplay Widget Blueprint
    1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/22f658dc6d7b9f4e8c5833a8171a379546da440a)
    2. Create a new Blueprint widget named `Gameplaywidget_WBP`. check `/All/Game/Widgets/Gameplays`.
    3. Attach this new gameplaywidget into CPlayerController_BP.
 
- 22,
