Shader "X Engine/Normal Shader"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
        _RuleTex ("Texture", 2D) = "white" {}
    }

    SubShader
    {
        Tags
        {
            "Queue"="Transparent"
            "RenderType"="Transparent"
            "IgnoreProjector"="True"
        }

        LOD 100
        ZWrite Off
        //ZTest Always
        Blend SrcAlpha OneMinusSrcAlpha
        BlendOp Add

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
                float4 color : COLOR;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
                float4 color : COLOR;
            };

            sampler2D _MainTex;
            sampler2D _RuleTex;

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = v.vertex;
                o.uv = v.uv;
                o.color = v.color;
                return o;
            }

            float4 frag (v2f i) : SV_Target
            {
                float4 col1 = tex2D(_MainTex, i.uv);
                float4 col2 = tex2D(_RuleTex, i.uv);
                col1.a = clamp((1.0 - col2.b) + (i.color.a * 2.0 - 1.0), 0.0, 1.0);
                return col1;
            }
            ENDCG
        }
    }
}
