import streamlit as st

st.set_page_config(page_title="Controller", layout="wide", page_icon="favicon.ico")
st.title("CS1600 Water Gun Controller")
hide_streamlit_style = """
            <style>
            #MainMenu {visibility: hidden;}
            footer {visibility: hidden;}
            </style>
            """

action = "NO ACTION"

col1, col2, col3 = st.columns([1, 1, 1])

with col1:
    if st.button("Turn Left"):
        action = "LEFT"
with col2:
    if st.button("STOP"):
        action = "NO ACTION"
with col3:
    if st.button("Turn Right"):
        action = "RIGHT"

with col2:
    st.write(f'Plate action taken: {action}')
